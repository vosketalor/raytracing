#pragma once
#include "Scene.h"

class Scene1 final : public Scene {
    void createShapes() override;
    void createLights() override;
};

