#include "Texture.h"
#include <iostream>
#include <fstream>
#include <algorithm>

// Utilisation de stb_image pour le support JPEG/PNG/BMP/TGA/etc.
#include "../../../external/stb_image.h"

Texture::Texture(const std::string& textureFileName): width(0), height(0) {
    if (!loadFromFile(textureFileName)) {
        std::cerr << "Texture file not found or could not be loaded: " << textureFileName << std::endl;
    }
}

bool Texture::loadFromFile(const std::string& filename) {
    int originalChannels;
    unsigned char* tempData = stbi_load(filename.c_str(), &width, &height, &originalChannels, 0);

    if (!tempData) {
        std::cerr << "Failed to load image: " << filename << " - " << stbi_failure_reason() << std::endl;
        return false;
    }

    // Maintenant rechargez avec le bon nombre de canaux
    stbi_image_free(tempData);
    channels = originalChannels;
    data = stbi_load(filename.c_str(), &width, &height, &channels, channels);

    if (!data) {
        std::cerr << "Failed to reload image with correct channels" << std::endl;
        return false;
    }

    return true;
}

Texture::~Texture() {
    if (data) {
        stbi_image_free(data);
        data = nullptr;
    }
}