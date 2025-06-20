#include "Texture.h"
#include <iostream>
#include <fstream>
#include <algorithm>

// Utilisation de stb_image pour le support JPEG/PNG/BMP/TGA/etc.
#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"

// Texture::Texture(const std::string& textureFileName)
//     : textureFileName(textureFileName), width(0), height(0), isLoaded(false) {
//
//     if (!loadFromFile(textureFileName)) {
//         std::cerr << "Texture file not found or could not be loaded: " << textureFileName << std::endl;
//
//         // Créer une texture par défaut (damier 2x2)
//         width = 2;
//         height = 2;
//         textureData.resize(height);
//         for (int y = 0; y < height; ++y) {
//             textureData[y].resize(width);
//             for (int x = 0; x < width; ++x) {
//                 // Motif damier simple : blanc et magenta
//                 if ((x + y) % 2 == 0) {
//                     textureData[y][x] = Vector3(1.0, 1.0, 1.0); // Blanc
//                 } else {
//                     textureData[y][x] = Vector3(1.0, 0.0, 1.0); // Magenta
//                 }
//             }
//         }
//         isLoaded = true; // Texture par défaut créée avec succès
//     }
// }
//
// Vector3 Texture::getTextureColor(const Vector2& uv) const {
//     if (!isLoaded || width == 0 || height == 0) {
//         return Vector3(1.0, 0.0, 1.0); // Magenta pour indiquer une erreur
//     }
//
//     // Clamp des coordonnées UV dans [0, 1]
//     const float u = std::max(0.0, std::min(1.0, uv.x()));
//     const float v = std::max(0.0, std::min(1.0, uv.y()));
//
//     // Conversion en coordonnées de pixel
//     int pixelU = static_cast<int>(u * (width - 1));
//     int pixelV = static_cast<int>(v * (height - 1));
//
//     // Assurer que les indices sont dans les limites
//     pixelU = std::max(0, std::min(width - 1, pixelU));
//     pixelV = std::max(0, std::min(height - 1, pixelV));
//
//     return textureData[pixelV][pixelU];
// }
//
// void Texture::scale(float scale) {
//     if (!isLoaded || scale <= 0.0) {
//         return;
//     }
//
//     int newWidth = static_cast<int>(width * scale);
//     int newHeight = static_cast<int>(height * scale);
//
//     if (newWidth <= 0 || newHeight <= 0) {
//         return;
//     }
//
//     // Créer la nouvelle texture redimensionnée
//     std::vector<std::vector<Vector3>> scaledTextureData(newHeight);
//     for (int y = 0; y < newHeight; ++y) {
//         scaledTextureData[y].resize(newWidth);
//         for (int x = 0; x < newWidth; ++x) {
//             // Échantillonnage du pixel le plus proche
//             int srcX = static_cast<int>(x / scale);
//             int srcY = static_cast<int>(y / scale);
//
//             // Assurer que les indices source sont valides
//             srcX = std::max(0, std::min(width - 1, srcX));
//             srcY = std::max(0, std::min(height - 1, srcY));
//
//             scaledTextureData[y][x] = textureData[srcY][srcX];
//         }
//     }
//
//     // Remplacer l'ancienne texture
//     textureData = std::move(scaledTextureData);
//     width = newWidth;
//     height = newHeight;
// }
//
// bool Texture::loadFromFile(const std::string& filename) {
//     int channels;
//     unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);
//
//     if (!data) {
//         std::cerr << "Failed to load image: " << filename << " - " << stbi_failure_reason() << std::endl;
//         return false;
//     }
//
//     // Convertir les données en format Vector3
//     textureData.resize(height);
//     for (int y = 0; y < height; ++y) {
//         textureData[y].resize(width);
//         for (int x = 0; x < width; ++x) {
//             int index = (y * width + x) * 3;  // 3 canaux RGB
//
//             // Normaliser les valeurs [0-255] vers [0-1]
//             const float r = static_cast<float>(data[index]) / 255.0;
//             const float g = static_cast<float>(data[index + 1]) / 255.0;
//             const float b = static_cast<float>(data[index + 2]) / 255.0;
//
//             textureData[y][x] = Vector3(r, g, b);
//         }
//     }
//
//     // Libérer la mémoire allouée par stb_image
//     stbi_image_free(data);
//
//     isLoaded = true;
//     return true;
// }

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