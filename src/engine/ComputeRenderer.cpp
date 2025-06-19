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
      pickSSBO(0) {
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
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glGenBuffers(1, &sceneDataSSBO);
    glGenBuffers(1, &lightDataSSBO);
    glGenBuffers(1, &materialDataSSBO);
    glGenBuffers(1, &bvhDataSSBO);
    glGenBuffers(1, &pickSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pickSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), nullptr, GL_STATIC_DRAW);

    updateSceneData();

    // glGenTextures(1, &textureAtlas);
    // glBindTexture(GL_TEXTURE_2D, textureAtlas);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasPixels);

    // glBindImageTexture(6, textureAtlas, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
}

void ComputeRenderer::updateSceneData() {
    std::vector<GPU::GPUShapeData> gpuShapes;
    for (const auto& shape : scene->getShapes()) {
        gpuShapes.push_back(shape->toGPU());
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sceneDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuShapes.size() * sizeof(GPU::GPUShapeData),
                 gpuShapes.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sceneDataSSBO);

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
            gpuMaterials.push_back(mat.toGPU());
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
    if (outputTexture) {
        glDeleteTextures(1, &outputTexture);
    }

    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);  // détacher texture

    // Bind l'image pour le compute shader
    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    updateUniforms();

    glUseProgram(shaderProgram);

    int groupsX = (width + 15) / 16;
    int groupsY = (height + 15) / 16;
    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(groupsX, groupsY, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, outputTexture);
    std::vector<float> pixels(width * height * 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    frameBuffer.resize(width * height);
    // for (int i = 0; i < width * height; i++) {
    //     frameBuffer[i] = Vector3(pixels[i*4], pixels[i*4+1], pixels[i*4+2]);
    // }
    std::for_each(
        std::execution::par,
        frameBuffer.begin(),
        frameBuffer.end(),
        [&](Vector3& color) {
            const size_t i = &color - frameBuffer.data();
            color = Vector3(pixels[i*4], pixels[i*4+1], pixels[i*4+2]);
        }
    );
}

void ComputeRenderer::setCamera(const Camera& camera) {
    camera_ = camera;
}

void ComputeRenderer::cleanup() {
    if (outputTexture) glDeleteTextures(1, &outputTexture);
    if (sceneDataSSBO) glDeleteBuffers(1, &sceneDataSSBO);
    if (lightDataSSBO) glDeleteBuffers(1, &lightDataSSBO);
    if (materialDataSSBO) glDeleteBuffers(1, &materialDataSSBO);
    if (bvhDataSSBO) glDeleteBuffers(1, &bvhDataSSBO);
    if (pickSSBO) glDeleteBuffers(1, &pickSSBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (pickShaderProgram) glDeleteProgram(pickShaderProgram);
    if (computeShader) glDeleteShader(computeShader);
    if (pickShader) glDeleteShader(pickShader);
}