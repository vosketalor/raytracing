// #pragma once
// #include "Shape.h"
// // #include "BoundingBox.h"
// #include <cmath>
// #include <limits>
//
// class Plane : public Shape {
// protected:
//     Vector3 normal;
//     double distance;
//
// public:
//     // Constructors
//     Plane(const Vector3& n, const double d);
//     Plane(const Vector3& P, const Vector3& n);
//     Plane(const Vector3& C, const Vector3& v1, const Vector3& v2);
//
//     // Shape interface implementation
//     Intersection getIntersection(const Vector3& P, const Vector3& v) const override;
//     // void scale(double scale) override;
//     // void rotate(double angle, const Vector3& axis) override;
//     void setBoundingBox() override;
//     Vector2 getTextureCoordinates(const Vector3& intersection) const override;
//
//     double getDistanceNearestEdge(const Vector3& P, const Camera& camera) const override;
// };

#include "Shape.h"

class Plane : public Shape {
public :
    glm::vec3 normal;
    float distance;

    Plane(const glm::vec3& n, const float d);
    Plane(const glm::vec3& P, const glm::vec3& n);
    Plane(const glm::vec3& C, const glm::vec3& v1, const glm::vec3& v2);
    GPU::GPUShapeData toGPU(Scene* scene) const override;
    void setBoundingBox() override;
};
