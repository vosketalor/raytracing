#pragma once

#include "Texture.h"
#include "SubTexture.h"
#include <unordered_map>
#include <string>

class TextureAtlas : public Texture {
public:
    TextureAtlas();
    ~TextureAtlas();

    bool addTexture(const std::string& path);
    const SubTexture* getSubTexture(const std::string& path) const;
    int getIndex(const std::string& path) const;

private:
    int currentY;
    std::unordered_map<std::string, SubTexture> subTextures;
    int num;
    std::unordered_map<std::string, int> indexes;

    void blit(const unsigned char* src, int srcWidth, int srcHeight, int destX, int destY) const;
    void resizeAtlas(int newWidth, int newHeight);
};
