#include "TextureAtlas.h"
#include "../../../external/stb_image.h"
#include <iostream>
#include <cstring>

TextureAtlas::TextureAtlas() : Texture(), currentY(0), num(0) {
    channels = 4; // RGBA
    width = 0;
    height = 0;
    data = nullptr;
}

TextureAtlas::~TextureAtlas() {
    if (data) {
        stbi_image_free(data);
        data = nullptr;
    }
}

bool TextureAtlas::addTexture(const std::string& path) {
    if (subTextures.find(path) != subTextures.end()) {
        return false; // déjà ajouté
    }

    int texWidth, texHeight, texChannels;
    unsigned char* texData = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, channels);
    if (!texData) {
        std::cerr << "Failed to load: " << path << " — " << stbi_failure_reason() << std::endl;
        return false;
    }
    std::cout << "Texture " << path << " loaded." << std::endl;

    const int newWidth = std::max(width, texWidth);
    const int newHeight = currentY + texHeight;

    resizeAtlas(newWidth, newHeight);

    blit(texData, texWidth, texHeight, 0, currentY);

    subTextures[path] = {0, currentY, texWidth, texHeight};

    currentY += texHeight;

    indexes[path] = num;
    ++num;

    stbi_image_free(texData);
    return true;
}

void TextureAtlas::resizeAtlas(int newWidth, int newHeight) {
    if (newWidth <= width && newHeight <= height) return;

    const int allocWidth = std::max(width, newWidth);
    const int allocHeight = std::max(height, newHeight);
    const int newSize = allocWidth * allocHeight * channels;

    unsigned char* newData = static_cast<unsigned char*>(malloc(newSize));
    std::memset(newData, 0, newSize);

    if (data) {
        for (int y = 0; y < height; ++y) {
            std::memcpy(
                newData + y * allocWidth * channels,
                data + y * width * channels,
                width * channels
            );
        }
        stbi_image_free(data);
    }

    data = newData;
    width = allocWidth;
    height = allocHeight;
}

void TextureAtlas::blit(const unsigned char* src, const int srcWidth, const int srcHeight, const int destX, const int destY) const
{
    for (int y = 0; y < srcHeight; ++y) {
        std::memcpy(
            data + ((destY + y) * width + destX) * channels,
            src + y * srcWidth * channels,
            srcWidth * channels
        );
    }
}

const SubTexture* TextureAtlas::getSubTexture(const std::string& path) const {
    const auto it = subTextures.find(path);
    return (it != subTextures.end()) ? &it->second : nullptr;
}

int TextureAtlas::getIndex(const std::string& path) const
{
    const auto it = indexes.find(path);
    return (it != indexes.end()) ? it->second : -1;
}

