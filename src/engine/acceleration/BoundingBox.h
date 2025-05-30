#pragma once

#include "../Vector.h"
#include "../Intersection.h"
#include "../scenes/Scene.h"

/**
 * Représente une boîte englobante alignée sur les axes (AABB).
 */
class BoundingBox : public Shape {
private:
    Vector3 min;
    Vector3 max;

public:
    BoundingBox(const Vector3& min, const Vector3& max);

    Vector3 getMin() const;
    Vector3 getMax() const;

    Intersection getIntersection(const Vector3& P, const Vector3& v) const override;

    // void scale(double scale) override;
    // void rotate(double angle, const Vector3& axis) override;
    void setBoundingBox() override;
    Vector2 getTextureCoordinates(const Vector3& intersection) const override;

    bool contains(const Vector3& P) const;
};
