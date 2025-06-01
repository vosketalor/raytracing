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
    Camera camera_;

public:
    int width, height;

    explicit Renderer(Scene* scene, const Camera& camera, const int& width, const int& height) :
        scene(scene), bvh_(scene->getShapes()), camera_(camera), width(width), height(height)
    {
    };

    void render(std::vector<Vector3> &frameBuffer) const;

    const Shape* getShape(const double& x, const double& y);

    void setCamera(const Camera& camera);

private:
    Vector3 getPixelColor(const Vector3& P, const Vector3& v, const int& order, unsigned int* seed) const;
    Intersection findNearestIntersection(const Vector3& P, const Vector3& v) const;
    Vector3 computeLighting(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, unsigned int* seed) const;
    bool isInShadow(const Vector3& shadowOrigin, const Vector3& shadowRayDir, double lightDistance) const;
    Vector3 computeShadowAttenuation(const Vector3& origin, const Vector3& dir, double lightDist) const;
    Vector3 computeDiffuse(const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const LightSource& lightSource, const Vector3& shadowOrigin, const Vector3& shadowRayDir) const;
    Vector3 computeSpecular(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const LightSource& lightSource) const;
    double computeAttenuation(const double& distance) const;
    Vector3 perturbVector(const Vector3 &direction, const Vector3 &normal, double roughness, unsigned int* seed) const;
    Vector3 computeReflection(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const int& order, unsigned int* seed) const;
    Vector3 computeRefraction(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const int& order, unsigned int* seed) const;
};
