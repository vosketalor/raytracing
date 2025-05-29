#pragma once

#include <vector>
#include <cstdint>

#include "Intersection.h"
#include "scenes/Scene.h"

class Renderer
{
    Scene* scene;

public:
    explicit Renderer(Scene* scene) : scene(scene) {};

    void render(int width, int height, std::vector<Vector3> &frameBuffer);

private:
    Vector3 getPixelColor(const Vector3& P, const Vector3& v);
    Intersection findNearestIntersection(const Vector3& P, const Vector3& v) const;
};
