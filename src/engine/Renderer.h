#pragma once

#include <vector>
#include <cstdint>

#include "Camera.h"
#include "acceleration/BVHNode.h"
#include "Intersection.h"
#include "scenes/Scene.h"

class Renderer
{
    Scene* scene;
    BVHNode bvh_;

public:
    explicit Renderer(Scene* scene) : scene(scene), bvh_(scene->getShapes()) {};

    void render(int width, int height, std::vector<Vector3> &frameBuffer, const Camera& camera) const;

private:
    Vector3 getPixelColor(const Vector3& P, const Vector3& v, const int& order) const;
    Intersection findNearestIntersection(const Vector3& P, const Vector3& v) const;
    Vector3 computeLighting(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape) const;
    bool isInShadow(const Vector3& shadowOrigin, const Vector3& shadowRayDir, double lightDistance) const;
    Vector3 computeDiffuse(const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const LightSource& lightSource, const Vector3& shadowOrigin, const Vector3& shadowRayDir) const;
    Vector3 computeSpecular(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const LightSource& lightSource) const;
    double computeAttenuation(const double& distance) const;
    Vector3 computeReflection(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const int& order) const;
    Vector3 computeRefraction(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const int& order) const;
};
