#include "ComputeRenderer.h"

#include <execution>
#include <iostream>
#include <fstream>
#include <sstream>

#include "acceleration/GPUBVHNode.h"

ComputeRenderer::ComputeRenderer(Scene* scene, const Camera& camera, int width, int height)
    : scene(scene), camera_(camera), bvh_(scene->getShapes(), 0),
      computeShader(0), pickShader(0),
      shaderProgram(0), pickShaderProgram(0), outputTexture(0),
      sceneDataSSBO(0), lightDataSSBO(0), materialDataSSBO(0), bvhDataSSBO(0),
      pickSSBO(0), textureSSBO(0), textureAtlas(0) {
    prefs.load();
    reflectionsEnabled = prefs.get("reflectionsEnabled", false);
    refractionsEnabled = prefs.get("refractionsEnabled", false);
    specularEnabled    = prefs.get("specularEnabled", false);
    attenuationEnabled = prefs.get("attenuationEnabled", false);
    shadowsEnabled     = prefs.get("shadowsEnabled", false);
    immediateEffect    = prefs.get("immediateEffect", false);
    fresnelEnabled     = prefs.get("fresnelEnabled", false);
    roughnessEnabled   = prefs.get("roughnessEnabled", false);
    bvhEnabled         = prefs.get("bvhEnabled", false);
    oversamplingEnabled = prefs.get("oversamplingEnabled", false);
    oversamplingFactor = prefs.get("oversamplingFactor", 2);
    this->width        = prefs.get("width", 512);
    this->height       = prefs.get("height", 384);
    std::cout << prefs << std::endl;
}

ComputeRenderer::~ComputeRenderer() {
    cleanup();
}

std::string ComputeRenderer::loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <algorithm>

std::string ComputeRenderer::loadShaderWithIncludes(const std::string& filePath, const std::string& basePath = "res/shaders/",
                                                    std::unordered_set<std::string>* includedFiles = nullptr) {
    if (!includedFiles) {
        std::unordered_set<std::string> localSet;
        return loadShaderWithIncludes(filePath, basePath, &localSet);
    }
    if (includedFiles->count(basePath + filePath)) {
        // Déjà inclus, on évite la récursion infinie
        return "";
    }
    std::cout << "Included shader : " << basePath << filePath << std::endl;
    includedFiles->insert(basePath + filePath);

    std::ifstream file(basePath + filePath);
    if (!file.is_open()) throw std::runtime_error("Shader not found: " + basePath + filePath);

    std::string line;
    std::stringstream output;

    while (std::getline(file, line)) {
        // Supprime espaces début/fin
        auto start = line.find_first_not_of(" \t");
        auto end = line.find_last_not_of(" \t");
        std::string trimmedLine = (start == std::string::npos) ? "" : line.substr(start, end - start + 1);

        if (trimmedLine.find("#include") == 0) {
            size_t firstQuote = trimmedLine.find('\"');
            size_t lastQuote = trimmedLine.find_last_of('\"');
            if (firstQuote != std::string::npos && lastQuote != std::string::npos && lastQuote > firstQuote) {
                std::string includePath = trimmedLine.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                output << loadShaderWithIncludes(includePath, basePath, includedFiles);
            } else {
                throw std::runtime_error("Invalid #include directive: " + line);
            }
        } else if (!trimmedLine.empty() && trimmedLine.find("//") != 0) {
            output << line << '\n';
        }
    }
    file.close();

    return output.str();
}

void saveShaderToFile(const std::string& shaderSource, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier pour écriture : " + filename);
    }
    outFile << shaderSource;
    outFile.close();
}


bool ComputeRenderer::initialize() {
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    if (!GLEW_ARB_compute_shader) {
        std::cerr << "Compute shaders not supported" << std::endl;
        return false;
    }

    const std::string shaderSource = loadShaderWithIncludes("shader.glsl");
    saveShaderToFile(shaderSource, "whole_shader.glsl");

    const std::string pickSource = loadShaderWithIncludes("shader_pick.glsl");
    saveShaderToFile(pickSource, "whole_shader_pick.glsl");

    if (shaderSource.empty() || pickSource.empty()) {
        return false;
    }

    if (!loadComputeShader(computeShader, shaderProgram, shaderSource)) return false;
    if (!loadComputeShader(pickShader, pickShaderProgram, pickSource)) return false;

    setupBuffers();

    return true;
}

bool ComputeRenderer::loadComputeShader(GLuint& shader, GLuint& program, const std::string& shaderSource) {
    shader = glCreateShader(GL_COMPUTE_SHADER);
    const char* source = shaderSource.c_str();
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Compute shader compilation failed:\n" << infoLog << std::endl;
        return false;
    }

    program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        return false;
    }

    return true;
}

int ComputeRenderer::pick(const int mouseX, const int mouseY) const {
    // 1) Réinitialiser le SSBO de pick
    const int clearVal = -1;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pickSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &clearVal);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sceneDataSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightDataSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, materialDataSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bvhDataSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, pickSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, textureSSBO);

    // 2) Choix du programme de picking
    glUseProgram(pickShaderProgram);

    // 2a) Remplissage des uniforms de caméra et clic
    updateCameraUniforms(pickShaderProgram);

    setUniform1i(pickShaderProgram, "numShapes", static_cast<int>(scene->getShapes().size()));
    setUniform1i(pickShaderProgram, "numBVHNodes", static_cast<int>(this->bvh_.toGPU(scene->getShapes()).size()));
    setUniform2i(pickShaderProgram, "u_ClickCoord", mouseX, mouseY);

    // 3) Dispatch et synchronisation
    int groupsX = (width + 15) / 16;  // ou (width + 0) / 1 si vous gardez local_size 1
    int groupsY = (height + 15) / 16; // ou (height + 0) / 1 si vous gardez local_size 1
    glDispatchCompute(groupsX, groupsY, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // 4) Lecture CPU
    int pickedID = -1;
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &pickedID);
    return pickedID;
}

void ComputeRenderer::setupBuffers() {
    // === Création de la texture de sortie ===
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // === Création des SSBOs ===
    glGenBuffers(1, &sceneDataSSBO);
    glGenBuffers(1, &lightDataSSBO);
    glGenBuffers(1, &materialDataSSBO);
    glGenBuffers(1, &bvhDataSSBO);
    glGenBuffers(1, &pickSSBO);
    glGenBuffers(1, &textureSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pickSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // === Chargement de la texture principale ===
    if (!scene->texture_atlas.data || scene->texture_atlas.width == 0 || scene->texture_atlas.height == 0) {
        std::cerr << "ERREUR: Données de texture invalides" << std::endl;
        return;
    }

    // === Création de la texture atlas ===
    glGenTextures(1, &textureAtlas);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);

    // Déterminer le format basé sur les canaux réels
    GLenum format, internalFormat;
    switch(scene->texture_atlas.channels) {
    case 1:
        format = GL_RED;
        internalFormat = GL_R8;
        break;
    case 3:
        format = GL_RGB;
        internalFormat = GL_RGB8;
        break;
    case 4:
        format = GL_RGBA;
        internalFormat = GL_RGBA8;
        break;
    default:
        std::cerr << "Format de texture non supporté: " << scene->texture_atlas.channels << " canaux" << std::endl;
        return;
    }

    // Configuration des paramètres AVANT glTexImage2D
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Création de la texture
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                 scene->texture_atlas.width, scene->texture_atlas.height, 0,
                 format, GL_UNSIGNED_BYTE, scene->texture_atlas.data);

    // Vérification des erreurs
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "ERREUR OpenGL: 0x" << std::hex << error << std::dec << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    updateSceneData();
}


void ComputeRenderer::updateSceneData() {
    std::vector<GPU::GPUShapeData> gpuShapes;
    std::vector<SubTexture> gpuTextures;
    for (const auto& shape : scene->getShapes()) {
        gpuShapes.push_back(shape->toGPU(this->scene));
        if (shape->hasTexture())
        {
            const SubTexture* subTex = scene->texture_atlas.getSubTexture(shape->getTexture());
            if (subTex != nullptr)
            {
                gpuTextures.push_back(*subTex);
            }
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sceneDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuShapes.size() * sizeof(GPU::GPUShapeData),
                 gpuShapes.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sceneDataSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, textureSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuTextures.size() * sizeof(SubTexture),
                 gpuTextures.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, textureSSBO);

    std::vector<GPU::GPULightSource> gpuLights;
    for (const auto& light : scene->getLightSources()) {
        gpuLights.push_back(light->toGPU());
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuLights.size() * sizeof(GPU::GPULightSource),
                 gpuLights.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightDataSSBO);

    std::vector<GPU::GPUMaterial> gpuMaterials;
    for (const auto& mat : Shape::materials) {
            gpuMaterials.push_back(mat.toGPU(scene));
        if (mat.hasAlbedoMap())
        {
            const SubTexture* subTex = scene->texture_atlas.getSubTexture(mat.getAlbedoMap());
            if (subTex != nullptr)
            {
                gpuTextures.push_back(*subTex);
            }
        }

        // Pour l'Ambient Occlusion Map
        if (mat.hasAmbientOcclusionMap()) {
            const SubTexture* subTex = scene->texture_atlas.getSubTexture(mat.getAmbientOcclusionMap());
            if (subTex != nullptr)
            {
                gpuTextures.push_back(*subTex);
            }
        }

        // Pour la Normal Map
        if (mat.hasNormalMap()) {
            const SubTexture* subTex = scene->texture_atlas.getSubTexture(mat.getNormalMap());
            if (subTex != nullptr)
            {
                gpuTextures.push_back(*subTex);
            }
        }

        // Pour la Roughness Map
        if (mat.hasRoughnessMap()) {
            const SubTexture* subTex = scene->texture_atlas.getSubTexture(mat.getRoughnessMap());
            if (subTex != nullptr)
            {
                gpuTextures.push_back(*subTex);
            }
        }

        // Pour la Metalness Map
        if (mat.hasMetalnessMap()) {
            const SubTexture* subTex = scene->texture_atlas.getSubTexture(mat.getMetalnessMap());
            if (subTex != nullptr)
            {
                gpuTextures.push_back(*subTex);
            }
        }

        // Pour la Height Map
        if (mat.hasHeightMap()) {
            const SubTexture* subTex = scene->texture_atlas.getSubTexture(mat.getHeightMap());
            if (subTex != nullptr)
            {
                gpuTextures.push_back(*subTex);
            }
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuMaterials.size() * sizeof(GPU::GPUMaterial),
                 gpuMaterials.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, materialDataSSBO);

    std::vector<GPU::GPUBVHNode> gpuBVH = this->bvh_.toGPU(scene->getShapes());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuBVH.size() * sizeof(GPU::GPUBVHNode),
                 gpuBVH.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bvhDataSSBO);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, pickSSBO);
}

void ComputeRenderer::setUniform3f(GLuint program, const char* name, float x, float y, float z) const {
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) {
        std::cerr << "Uniform '" << name << "' not found!" << std::endl;
    } else {
        glUniform3f(loc, x, y, z);
    }
}

void ComputeRenderer::setUniform2f(GLuint program, const char* name, float x, float y) const {
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) {
        std::cerr << "Uniform '" << name << "' not found!" << std::endl;
    } else {
        glUniform2f(loc, x, y);
    }
}

void ComputeRenderer::setUniform1f(GLuint program, const char* name, float value) const {
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) {
        std::cerr << "Uniform '" << name << "' not found!" << std::endl;
    } else {
        glUniform1f(loc, value);
    }
}

void ComputeRenderer::setUniform1i(GLuint program, const char* name, int value) const {
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) {
        std::cerr << "Uniform '" << name << "' not found!" << std::endl;
    } else {
        glUniform1i(loc, value);
    }
}

void ComputeRenderer::setUniform2i(GLuint program, const char* name, int x, int y) const {
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) {
        std::cerr << "Uniform '" << name << "' not found!" << std::endl;
    } else {
        glUniform2i(loc, x, y);
    }
}

void ComputeRenderer::setUniformBool(GLuint program, const char* name, bool value) const {
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) {
        std::cerr << "Uniform '" << name << "' not found!" << std::endl;
    } else {
        glUniform1i(loc, value ? 1 : 0);  // true = 1, false = 0
    }
}

void ComputeRenderer::updateCameraUniforms(const GLuint& program) const
{
    const glm::vec3 pos = camera_.getPosition();
    const glm::vec3 dir = camera_.getDirection();
    const glm::vec3 right = camera_.getRight();
    const glm::vec3 up = camera_.getUp();

    setUniform3f(program, "cameraPos", pos.x, pos.y, pos.z);
    setUniform3f(program, "cameraDir", dir.x, dir.y, dir.z);
    setUniform3f(program, "cameraRight", right.x, right.y, right.z);
    setUniform3f(program, "cameraUp", up.x, up.y, up.z);
    setUniform1f(program, "fov", camera_.getFov());
    setUniform1f(program, "aspectRatio", static_cast<float>(width) / static_cast<float>(height));
    setUniform2i(program, "resolution", width, height);
}


void ComputeRenderer::updateUniforms() const {
    glUseProgram(shaderProgram);

    updateCameraUniforms(shaderProgram);

    // setUniform1i(shaderProgram, "numMaterials", static_cast<int>(Shape::materials.size()));
    setUniform1i(shaderProgram, "numShapes", static_cast<int>(scene->getShapes().size()));
    setUniform1i(shaderProgram, "numLights", static_cast<int>(scene->getLightSources().size()));
    //TODO : do better later
    setUniform1i(shaderProgram, "numBVHNodes", static_cast<int>(this->bvh_.toGPU(scene->getShapes()).size()));
    setUniform2f(shaderProgram, "textureAtlasSize", static_cast<float>(scene->texture_atlas.width), static_cast<float>(scene->texture_atlas.height));

    setUniformBool(shaderProgram, "reflectionsEnabled", reflectionsEnabled);
    setUniformBool(shaderProgram, "refractionsEnabled", refractionsEnabled);
    setUniformBool(shaderProgram, "specularEnabled", specularEnabled);
    setUniformBool(shaderProgram, "attenuationEnabled", attenuationEnabled);
    setUniformBool(shaderProgram, "shadowsEnabled", shadowsEnabled);
    setUniformBool(shaderProgram, "fresnelEnabled", fresnelEnabled);
    setUniformBool(shaderProgram, "roughnessEnabled", roughnessEnabled);
    setUniformBool(shaderProgram, "useBVH", bvhEnabled);
    setUniformBool(shaderProgram, "oversamplingEnabled", oversamplingEnabled);
    setUniform1i(shaderProgram, "oversamplingFactor", oversamplingFactor);

    setUniform1f(shaderProgram, "u_time", static_cast<float>(glfwGetTime()));

    setUniform1f(shaderProgram, "wireframeThickness", 0.05);
    setUniform3f(shaderProgram, "wireframeColor", 0, 1, 0);

    glm::vec3 skyColor = scene->getSkyColor();
    glm::vec3 ambient = scene->getAmbient();
    setUniform3f(shaderProgram, "skyColor", skyColor.x, skyColor.y, skyColor.z);
    setUniform3f(shaderProgram, "ambientColor", ambient.x, ambient.y, ambient.z);
}

void ComputeRenderer::render(std::vector<Vector3>& frameBuffer) {
    if (textureAtlas != 0 && glIsTexture(textureAtlas)) {
        glBindTexture(GL_TEXTURE_2D, textureAtlas);

        GLint width, height, internalFormat;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        std::cerr << "ERREUR: Texture atlas invalide!" << std::endl;
        return;
    }

    // === Recréation de la texture de sortie ===
    if (outputTexture) {
        glDeleteTextures(1, &outputTexture);
    }

    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // === Mise à jour des uniforms ===
    updateUniforms();

    // === Activation du programme shader ===
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    // === 5. Configuration de la texture atlas ===
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);

    // Vérifier le binding
    GLint boundTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);

    // === 6. Configuration de l'uniform ===
    GLint uniformLoc = glGetUniformLocation(shaderProgram, "textureAtlas");

    if (uniformLoc != -1) {
        glUniform1i(uniformLoc, 0);

        // Vérifier la valeur de l'uniform
        GLint uniformValue;
        glGetUniformiv(shaderProgram, uniformLoc, &uniformValue);
    } else {
        std::cerr << "ERREUR: Uniform 'textureAtlas' non trouvé dans le shader!" << std::endl;
    }

    // === 7. Vérification avant dispatch ===
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);

    // === Dispatch du compute shader ===
    int groupsX = (width + 15) / 16;
    int groupsY = (height + 15) / 16;
    glDispatchCompute(groupsX, groupsY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // === Lecture des résultats ===
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    std::vector<float> pixels(width * height * 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    // === Conversion des données ===
    frameBuffer.resize(width * height);
    std::for_each(
        std::execution::par,
        frameBuffer.begin(),
        frameBuffer.end(),
        [&](Vector3& color) {
            const size_t i = &color - frameBuffer.data();
            color = Vector3(pixels[i*4], pixels[i*4+1], pixels[i*4+2]);
    });

    // === Nettoyage ===
    glActiveTexture(GL_TEXTURE0);
}

void ComputeRenderer::setCamera(const Camera& camera) {
    camera_ = camera;
}

void ComputeRenderer::cleanup() {
    if (outputTexture) glDeleteTextures(1, &outputTexture);
    if (textureAtlas) glDeleteTextures(1, &textureAtlas);
    if (sceneDataSSBO) glDeleteBuffers(1, &sceneDataSSBO);
    if (lightDataSSBO) glDeleteBuffers(1, &lightDataSSBO);
    if (materialDataSSBO) glDeleteBuffers(1, &materialDataSSBO);
    if (bvhDataSSBO) glDeleteBuffers(1, &bvhDataSSBO);
    if (pickSSBO) glDeleteBuffers(1, &pickSSBO);
    if (textureSSBO) glDeleteBuffers(1, &textureSSBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (pickShaderProgram) glDeleteProgram(pickShaderProgram);
    if (computeShader) glDeleteShader(computeShader);
    if (pickShader) glDeleteShader(pickShader);
}