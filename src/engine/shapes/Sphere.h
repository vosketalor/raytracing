#pragma once
#include "Shape.h"


//
// class Sphere : public Shape {
// private:
//     Vector3 center;
//     float radius;
//
// public:
//     Sphere(const Vector3& P, float radius);
//
//     // Intersection getIntersection(const Vector3& P, const Vector3& v) const override;
//     // Vector3 getNormal(const Vector3& P) const;
//
//     // void scale(float scale) override;
//     // void rotate(float angle, const Vector3& axis) override;
//
//     // void setBoundingBox() override;
//     // Vector2 getTextureCoordinates(const Vector3& intersection) const override;
//     //
//     // double getDistanceNearestEdge(const Vector3& P, const Camera& camera) const override;
// };

class Sphere : public Shape
{
private:
    Vector3 center;
    float radius;

public:
    Sphere(const Vector3& P, float radius);
    GPU::GPUShapeData toGPUShapeData() const override;
};