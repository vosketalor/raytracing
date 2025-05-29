#include "Renderer.h"
#include <cstdlib>

void Renderer::render(int width, int height, std::vector<Renderer::Color>& frameBuffer) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            frameBuffer[y * width + x] = getPixelColor(x, y, width, height);
        }
    }
}

Renderer::Color Renderer::getPixelColor(int x, int y, int width, int height)
{
    uint8_t r = (x * 255) / width;
    uint8_t g = (y * 255) / height;
    uint8_t b = ((x + y) * 255) / (width + height);
    return {r, g, b};
}