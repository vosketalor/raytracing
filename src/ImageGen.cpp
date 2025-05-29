#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>

#include "engine/Renderer.h"

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <width> <height> <filename>" << std::endl;
        return 1;
    }

    int width = std::atoi(argv[1]);
    int height = std::atoi(argv[2]);
    std::string filename = argv[3];

    Renderer renderer;
    std::vector<Renderer::Color> frameBuffer(width * height);
    renderer.render(width, height, frameBuffer);

    std::ofstream image(filename);
    if (!image.is_open())
    {
        std::cerr << "Error while opening file: " << filename << std::endl;
        return 1;
    }

    image << "P3\n"
          << width << " " << height << "\n255\n";

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Renderer::Color &c = frameBuffer[y * width + x];
            image << (int)c.r << " " << (int)c.g << " " << (int)c.b << "\n";
        }
        image << "\n";
    }

    image.close();
    std::cout << "Image generated: " << filename << std::endl;

    return 0;
}