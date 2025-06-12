#pragma once
#include "Scene.h"

class SceneGPU : public Scene{
public:
    SceneGPU() = default;
    ~SceneGPU() override = default;

    void createShapes() override;
    void createLights() override;
};

