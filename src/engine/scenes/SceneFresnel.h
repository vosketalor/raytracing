#pragma once
#include "Scene.h"

class SceneFresnel final : public Scene {

public:
    SceneFresnel() = default;
    ~SceneFresnel() override = default;

    void createShapes() override;
    void createLights() override;
};

