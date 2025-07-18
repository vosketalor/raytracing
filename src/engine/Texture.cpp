#include "Texture.h"
#include <iostream>
#include <fstream>
#include <algorithm>

// Utilisation de stb_image pour le support JPEG/PNG/BMP/TGA/etc.
#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"

Texture::Texture(const std::string& textureFileName)
    : textureFileName(textureFileName), width(0), height(0), isLoaded(false) {

    if (!loadFromFile(textureFileName)) {
        std::cerr << "Texture file not found or could not be loaded: " << textureFileName << std::endl;

        // Créer une texture par défaut (damier 2x2)
        width = 2;
        height = 2;
        textureData.resize(height);
        for (int y = 0; y < height; ++y) {
            textureData[y].resize(width);
            for (int x = 0; x < width; ++x) {
                // Motif damier simple : blanc et magenta
                if ((x + y) % 2 == 0) {
                    textureData[y][x] = Vector3(1.0, 1.0, 1.0); // Blanc
                } else {
                    textureData[y][x] = Vector3(1.0, 0.0, 1.0); // Magenta
                }
            }
        }
        isLoaded = true; // Texture par défaut créée avec succès
    }
}

Vector3 Texture::getTextureColor(const Vector2& uv) const {
    if (!isLoaded || width == 0 || height == 0) {
        return Vector3(1.0, 0.0, 1.0); // Magenta pour indiquer une erreur
    }

    // Clamp des coordonnées UV dans [0, 1]
    double u = std::max(0.0, std::min(1.0, uv.x()));
    double v = std::max(0.0, std::min(1.0, uv.y()));

    // Conversion en coordonnées de pixel
    int pixelU = static_cast<int>(u * (width - 1));
    int pixelV = static_cast<int>(v * (height - 1));

    // Assurer que les indices sont dans les limites
    pixelU = std::max(0, std::min(width - 1, pixelU));
    pixelV = std::max(0, std::min(height - 1, pixelV));

    return textureData[pixelV][pixelU];
}

void Texture::scale(double scale) {
    if (!isLoaded || scale <= 0.0) {
        return;
    }

    int newWidth = static_cast<int>(width * scale);
    int newHeight = static_cast<int>(height * scale);

    if (newWidth <= 0 || newHeight <= 0) {
        return;
    }

    // Créer la nouvelle texture redimensionnée
    std::vector<std::vector<Vector3>> scaledTextureData(newHeight);
    for (int y = 0; y < newHeight; ++y) {
        scaledTextureData[y].resize(newWidth);
        for (int x = 0; x < newWidth; ++x) {
            // Échantillonnage du pixel le plus proche
            int srcX = static_cast<int>(x / scale);
            int srcY = static_cast<int>(y / scale);

            // Assurer que les indices source sont valides
            srcX = std::max(0, std::min(width - 1, srcX));
            srcY = std::max(0, std::min(height - 1, srcY));

            scaledTextureData[y][x] = textureData[srcY][srcX];
        }
    }

    // Remplacer l'ancienne texture
    textureData = std::move(scaledTextureData);
    width = newWidth;
    height = newHeight;
}

bool Texture::loadFromFile(const std::string& filename) {
    int channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);

    if (!data) {
        std::cerr << "Failed to load image: " << filename << " - " << stbi_failure_reason() << std::endl;
        return false;
    }

    // Convertir les données en format Vector3
    textureData.resize(height);
    for (int y = 0; y < height; ++y) {
        textureData[y].resize(width);
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;  // 3 canaux RGB

            // Normaliser les valeurs [0-255] vers [0-1]
            double r = static_cast<double>(data[index]) / 255.0;
            double g = static_cast<double>(data[index + 1]) / 255.0;
            double b = static_cast<double>(data[index + 2]) / 255.0;

            textureData[y][x] = Vector3(r, g, b);
        }
    }

    // Libérer la mémoire allouée par stb_image
    stbi_image_free(data);

    isLoaded = true;
    return true;
}