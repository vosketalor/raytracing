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
    Vector3 computeLighting(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape) const;
    bool isInShadow(const Vector3& shadowOrigin, const Vector3& shadowRayDir, double lightDistance) const;
    Vector3 computeDiffuse(const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const LightSource& lightSource, const Vector3& shadowOrigin, const Vector3& shadowRayDir) const;
    Vector3 computeSpecular(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const LightSource& lightSource) const;
};
