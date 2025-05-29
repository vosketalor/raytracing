#pragma once

#include <vector>
#include <cstdint>

class Renderer
{

public:
    Renderer() = default;

    struct Color
    {
        uint8_t r, g, b;
    };

    void render(int width, int height, std::vector<Color> &frameBuffer);

private:
    Color getPixelColor(int x, int y, int width, int height);
};