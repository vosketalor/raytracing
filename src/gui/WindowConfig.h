#pragma once
#include <string>

struct WindowConfig {
    int width = 1400;
    int height = 900;
    std::string title = "Raytracer";
    bool vsync = true;
    bool resizable = true;
};
