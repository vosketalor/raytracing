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
    return buffer.str();
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

    std::string shaderSource = loadShaderSource("res/shaders/simple_shader.glsl");
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
    std::vector<GPUShape> gpuShapes;
    for (const auto& shape : scene->getShapes()) {
        GPUShape gpuShape;

        gpuShape.center[0] = 0;
        gpuShape.center[1] = 0;
        gpuShape.center[2] = 0;
        gpuShape.radius = 1.0f;

        Vector3 color = shape->getColor();
        gpuShape.color[0] = color.x();
        gpuShape.color[1] = color.y();
        gpuShape.color[2] = color.z();

        gpuShape.type = 0;

        const Material& mat = shape->getMaterial();
        gpuShape.material[0] = mat.getRoughness();
        gpuShape.material[1] = mat.getMetallic();
        gpuShape.material[2] = mat.getTransparency();
        gpuShape.material[3] = mat.getReflectivity();

        gpuShapes.push_back(gpuShape);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sceneDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuShapes.size() * sizeof(GPUShape),
                 gpuShapes.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sceneDataSSBO);

    std::vector<GPULight> gpuLights;
    for (const auto& light : scene->getLightSources()) {
        GPULight gpuLight;

        Vector3 pos = light->getPosition();
        gpuLight.position[0] = pos.x();
        gpuLight.position[1] = pos.y();
        gpuLight.position[2] = pos.z();
        gpuLight.intensity = light->getIntensity();

        Vector3 diffuse = light->getColorDiffuse();
        gpuLight.colorDiffuse[0] = diffuse.x();
        gpuLight.colorDiffuse[1] = diffuse.y();
        gpuLight.colorDiffuse[2] = diffuse.z();

        Vector3 specular = light->getColorSpecular();
        gpuLight.colorSpecular[0] = specular.x();
        gpuLight.colorSpecular[1] = specular.y();
        gpuLight.colorSpecular[2] = specular.z();

        gpuLights.push_back(gpuLight);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuLights.size() * sizeof(GPULight),
                 gpuLights.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightDataSSBO);
}

void ComputeRenderer::updateCameraUniforms() {
    glUseProgram(shaderProgram);

    Vector3 pos = camera_.getPosition();
    Vector3 dir = camera_.getDirection();
    Vector3 right = camera_.getRight();
    Vector3 up = camera_.getUp();

    glUniform3f(glGetUniformLocation(shaderProgram, "cameraPos"), pos.x(), pos.y(), pos.z());
    glUniform3f(glGetUniformLocation(shaderProgram, "cameraDir"), dir.x(), dir.y(), dir.z());
    glUniform3f(glGetUniformLocation(shaderProgram, "cameraRight"), right.x(), right.y(), right.z());
    glUniform3f(glGetUniformLocation(shaderProgram, "cameraUp"), up.x(), up.y(), up.z());
    glUniform1f(glGetUniformLocation(shaderProgram, "fov"), camera_.getFov());
    glUniform1f(glGetUniformLocation(shaderProgram, "aspectRatio"), (float)width / height);

    GLint loc = glGetUniformLocation(shaderProgram, "resolution");
    if (loc == -1) {
        std::cerr << "Uniform 'resolution' not found!" << std::endl;
    } else
    {
        glUniform2i(loc, width, height);
    }

    // glUniform2i(glGetUniformLocation(shaderProgram, "resolution"), width, height);

    glUniform1i(glGetUniformLocation(shaderProgram, "numShapes"), scene->getShapes().size());
    glUniform1i(glGetUniformLocation(shaderProgram, "numLights"), scene->getLightSources().size());

    Vector3 skyColor = scene->getSkyColor();
    Vector3 ambient = scene->getAmbient();
    glUniform3f(glGetUniformLocation(shaderProgram, "skyColor"), skyColor.x(), skyColor.y(), skyColor.z());
    glUniform3f(glGetUniformLocation(shaderProgram, "ambientColor"), ambient.x(), ambient.y(), ambient.z());
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