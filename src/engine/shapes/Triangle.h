#pragma once
#include <vector>
#include "Plane.h"
#include "Vector.h"
#include "Intersection.h"

class Triangle : public Plane {
protected:
    Vector3 A, B, C;
    Vector3 uvA, uvB, uvC;
    bool hasUV = false;

public:
    Triangle(const Vector3& A, const Vector3& B, const Vector3& C);

    std::vector<Vector3> getVertices() const;

    Intersection getIntersection(const Vector3& P, const Vector3& v) const override;

    // void scale(double scale) override;
    //
    // void rotate(double angle, const Vector3& axis) override;

    void setBoundingBox() override;

    Vector2 getTextureCoordinates(const Vector3& intersection) const override;

    void setTextureCoordinates(const Vector3& a, const Vector3& b, const Vector3& c);

private:
    bool isInside(const Vector3& M) const;
};
