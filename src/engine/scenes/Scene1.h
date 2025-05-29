#pragma once
#include "Scene.h"

class Scene1 final : public Scene {

    public:
    Scene1() = default;
    ~Scene1() override = default;

    void createShapes() override;
    void createLights() override;
};

