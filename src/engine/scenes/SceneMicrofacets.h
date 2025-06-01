#pragma once
#include "Scene.h"

class SceneMicrofacets final : public Scene {

public:
    SceneMicrofacets() = default;
    ~SceneMicrofacets() override = default;

    void createShapes() override;
    void createLights() override;
};

