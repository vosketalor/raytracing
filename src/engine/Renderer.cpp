#include "Renderer.h"
#include <cstdlib>

void Renderer::render(const int width, const int height, std::vector<Renderer::Color>& frameBuffer) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            frameBuffer[y * width + x] = getPixelColor(x, y, width, height);
        }
    }
}

Renderer::Color Renderer::getPixelColor(const int x, const int y, const int width, const int height)
{
    const uint8_t r = (x * 255) / width;
    const uint8_t g = (y * 255) / height;
    const uint8_t b = ((x + y) * 255) / (width + height);
    return {r, g, b};
}