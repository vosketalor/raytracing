#pragma once

#include "../Vector.h"
#include "../scenes/Scene.h"

/**
 * Représente une boîte englobante alignée sur les axes (AABB).
 */
class BoundingBox : public Shape {
private:
    glm::vec3 min;
    glm::vec3 max;

public:
    BoundingBox(const glm::vec3& min, const glm::vec3& max);

    glm::vec3 getMin() const;
    glm::vec3 getMax() const;

    // Intersection getIntersection(const Vector3& P, const Vector3& v) const override;

    // void scale(double scale) override;
    // void rotate(double angle, const Vector3& axis) override;
    void setBoundingBox() override;
    // Vector2 getTextureCoordinates(const Vector3& intersection) const override;

    bool contains(const glm::vec3& P) const;

    // double getDistanceNearestEdge(const Vector3& P, const Camera& camera) const override;
};
