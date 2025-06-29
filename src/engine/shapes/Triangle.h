#pragma once
#include <vector>
#include "Plane.h"
#include "Vector.h"
// #include "Intersection.h"
//
class Triangle : public Plane {
protected:
    glm::vec3 A, B, C;
    // glm::vec3 uvA, uvB, uvC;
    // bool hasUV = false;

public:
    Triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C);

    std::vector<glm::vec3> getVertices() const;

    // Intersection getIntersection(const Vector3& P, const Vector3& v) const override;

    // void scale(double scale) override;
    //
    // void rotate(double angle, const Vector3& axis) override;

    void setBoundingBox() override;

    GPU::GPUShapeData toGPU(Scene* scene) const override;

    // Vector2 getTextureCoordinates(const Vector3& intersection) const override;

    // void setTextureCoordinates(const Vector3& a, const Vector3& b, const Vector3& c);

    // double getDistanceNearestEdge(const Vector3& P, const Camera& camera) const override;

private:
    // bool isInside(const Vector3& M) const;
};
