#pragma once

#include <vector>
#include <memory>
#include "BoundingBox.h"
#include "Vector.h"
#include "Intersection.h"
#include "shapes/Shape.h"

class BVHNode {
public:
    // Constructeur récursif
    explicit BVHNode(const std::vector<std::shared_ptr<Shape>>& shapes);

    // Intersection d'un rayon avec le nœud BVH
    Intersection getIntersection(const Vector3& P, const Vector3& d) const;

private:
    // Calcul du BoundingBox englobant toutes les formes
    BoundingBox computeBoundingBox(const std::vector<std::shared_ptr<Shape>>& shapes);

    std::shared_ptr<BoundingBox> boundingBox;
    std::shared_ptr<BVHNode> left = nullptr;
    std::shared_ptr<BVHNode> right = nullptr;
    std::shared_ptr<Shape> leafShape = nullptr;
};
