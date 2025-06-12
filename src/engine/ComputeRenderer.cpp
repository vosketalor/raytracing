#include "ComputeRenderer.h"

ComputeRenderer::ComputeRenderer(Scene* scene, const Camera& camera, int width, int height)
    : scene(scene), camera_(camera), width(width), height(height),
      computeShader(0), shaderProgram(0), outputTexture(0),
      sceneDataSSBO(0), lightDataSSBO(0), materialDataSSBO(0) {
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
    std::cout << "Loading shader from disk: " << basePath + filePath << std::endl;
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
    // std::cout << "Current percent to the number max of characters : " << (shaderSource.size() / shaderSource.max_size())* 100 << "%" << std::endl;

    if (shaderSource.empty()) {
        return false;
    }

    return loadComputeShader(shaderSource);
}

bool ComputeRenderer::loadComputeShader(const std::string& shaderSource) {
    computeShader = glCreateShader(GL_COMPUTE_SHADER);
    const char* source = shaderSource.c_str();
    glShaderSource(computeShader, 1, &source, NULL);
    glCompileShader(computeShader);

    GLint success;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        std::cerr << "Compute shader compilation failed:\n" << infoLog << std::endl;
        return false;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, computeShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        return false;
    }

    setupBuffers();
    return true;
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

    updateSceneData();
}

void ComputeRenderer::updateSceneData() {
    std::vector<GPU::GPUShapeData> gpuShapes;
    for (const auto& shape : scene->getShapes()) {
        GPU::GPUShapeData gpuShape;

        gpuShape = shape->toGPUShapeData();

        gpuShapes.push_back(gpuShape);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sceneDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuShapes.size() * sizeof(GPU::GPUShapeData),
                 gpuShapes.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sceneDataSSBO);

    std::cout << "Size shapes : " << gpuShapes.size() << std::endl;

    std::vector<GPULight> gpuLights;
    // for (const auto& light : scene->getLightSources()) {
    //     GPULight gpuLight;
    //
    //     Vector3 pos = light->getPosition();
    //     gpuLight.position[0] = pos.x();
    //     gpuLight.position[1] = pos.y();
    //     gpuLight.position[2] = pos.z();
    //     gpuLight.intensity = light->getIntensity();
    //
    //     Vector3 diffuse = light->getColorDiffuse();
    //     gpuLight.colorDiffuse[0] = diffuse.x();
    //     gpuLight.colorDiffuse[1] = diffuse.y();
    //     gpuLight.colorDiffuse[2] = diffuse.z();
    //
    //     Vector3 specular = light->getColorSpecular();
    //     gpuLight.colorSpecular[0] = specular.x();
    //     gpuLight.colorSpecular[1] = specular.y();
    //     gpuLight.colorSpecular[2] = specular.z();
    //
    //     gpuLights.push_back(gpuLight);
    // }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuLights.size() * sizeof(GPULight),
                 gpuLights.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightDataSSBO);
}

void setUniform3f(GLuint program, const char* name, float x, float y, float z) {
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) {
        std::cerr << "Uniform '" << name << "' not found!" << std::endl;
    } else {
        glUniform3f(loc, x, y, z);
    }
}

void setUniform1f(GLuint program, const char* name, float value) {
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) {
        std::cerr << "Uniform '" << name << "' not found!" << std::endl;
    } else {
        glUniform1f(loc, value);
    }
}

void setUniform1i(GLuint program, const char* name, int value) {
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) {
        std::cerr << "Uniform '" << name << "' not found!" << std::endl;
    } else {
        glUniform1i(loc, value);
    }
}

void setUniform2i(GLuint program, const char* name, int x, int y) {
    GLint loc = glGetUniformLocation(program, name);
    if (loc == -1) {
        std::cerr << "Uniform '" << name << "' not found!" << std::endl;
    } else {
        glUniform2i(loc, x, y);
    }
}

void ComputeRenderer::updateCameraUniforms() {
    glUseProgram(shaderProgram);

    Vector3 pos = camera_.getPosition();
    Vector3 dir = camera_.getDirection();
    Vector3 right = camera_.getRight();
    Vector3 up = camera_.getUp();

    setUniform3f(shaderProgram, "cameraPos", pos.x(), pos.y(), pos.z());
    setUniform3f(shaderProgram, "cameraDir", dir.x(), dir.y(), dir.z());
    setUniform3f(shaderProgram, "cameraRight", right.x(), right.y(), right.z());
    setUniform3f(shaderProgram, "cameraUp", up.x(), up.y(), up.z());
    setUniform1f(shaderProgram, "fov", camera_.getFov());
    setUniform1f(shaderProgram, "aspectRatio", (float)width / height);
    setUniform2i(shaderProgram, "resolution", width, height);

    setUniform1i(shaderProgram, "numShapes", scene->getShapes().size());
    setUniform1i(shaderProgram, "numLights", scene->getLightSources().size());

    Vector3 skyColor = scene->getSkyColor();
    Vector3 ambient = scene->getAmbient();
    setUniform3f(shaderProgram, "skyColor", skyColor.x(), skyColor.y(), skyColor.z());
    setUniform3f(shaderProgram, "ambientColor", ambient.x(), ambient.y(), ambient.z());
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

    updateCameraUniforms();

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
    for (int i = 0; i < width * height; i++) {
        frameBuffer[i] = Vector3(pixels[i*4], pixels[i*4+1], pixels[i*4+2]);
    }
}

void ComputeRenderer::setCamera(const Camera& camera) {
    camera_ = camera;
}

void ComputeRenderer::cleanup() {
    if (outputTexture) glDeleteTextures(1, &outputTexture);
    if (sceneDataSSBO) glDeleteBuffers(1, &sceneDataSSBO);
    if (lightDataSSBO) glDeleteBuffers(1, &lightDataSSBO);
    if (materialDataSSBO) glDeleteBuffers(1, &materialDataSSBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (computeShader) glDeleteShader(computeShader);
}