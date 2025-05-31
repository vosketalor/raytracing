#pragma once

#include <vector>
#include <string>
#include "Shape.h"
#include "Triangle.h"
#include "acceleration/BVHNode.h"
#include "Intersection.h"
#include "Vector.h"

class OBJ : public Shape {
public:
    OBJ(const std::string& objFileName, const Vector3& position);

    Intersection getIntersection(const Vector3& P, const Vector3& v) const override;
    // void scale(double scale) override;
    // void rotate(double angle, const Vector3& axis) override;
    void setBoundingBox() override;
    Vector2 getTextureCoordinates(const Vector3& intersection) const override;
    void rebuildBVH();

    void update();

private:
    std::vector<Triangle> triangles;
    BVHNode* bvh;

    Vector3 calculateCenter() const;
};

