#pragma once

#include <vector>
#include <cstdint>

#include "scenes/Scene.h"

class Renderer
{
    Scene* scene;

public:
    Renderer(Scene* scene) : scene(scene) {};

    struct Color
    {
        uint8_t r, g, b;
    };

    void render(int width, int height, std::vector<Color> &frameBuffer);

private:
    Color getPixelColor(int x, int y, int width, int height);
};
