#include "BVHNode.h"
#include "scenes/Scene.h"
#include <limits>
#include <algorithm>

BVHNode::BVHNode(const std::vector<std::shared_ptr<Shape>>& shapes)
{
    if (shapes.size() == 1) {
        leafShape = shapes[0];
        leafShape->setBoundingBox();
        boundingBox = leafShape->getBoundingBox();
    } else {
        boundingBox = std::make_shared<BoundingBox>(computeBoundingBox(shapes));

        Vector3 diag = boundingBox->getMax() - boundingBox->getMin();
        int axis = (diag[0] > diag[1])
            ? (diag[0] > diag[2] ? 0 : 2)
            : (diag[1] > diag[2] ? 1 : 2);

        auto sortedShapes = shapes;
        std::sort(sortedShapes.begin(), sortedShapes.end(),
            [axis](const std::shared_ptr<Shape>& a, const std::shared_ptr<Shape>& b) {
                const double centerA = (a->getBoundingBox()->getMin()[axis] + a->getBoundingBox()->getMax()[axis]) / 2.0;
                const double centerB = (b->getBoundingBox()->getMin()[axis] + b->getBoundingBox()->getMax()[axis]) / 2.0;
                return centerA < centerB;
            });

        size_t mid = sortedShapes.size() / 2;
        std::vector<std::shared_ptr<Shape>> leftShapes(sortedShapes.begin(), sortedShapes.begin() + mid);
        std::vector<std::shared_ptr<Shape>> rightShapes(sortedShapes.begin() + mid, sortedShapes.end());

        left = std::make_shared<BVHNode>(leftShapes);
        right = std::make_shared<BVHNode>(rightShapes);
    }
}

Intersection BVHNode::getIntersection(const Vector3& P, const Vector3& v) const {
    const Intersection boxHit = boundingBox->getIntersection(P, v);
    bool inside = boundingBox->contains(P);

    if (boxHit.lambda < Scene::EPSILON && !inside) {
        return Intersection();
    }

    if (leafShape != nullptr) {
        Intersection inter = leafShape->getIntersection(P, v);
        if (inter.lambda >= Scene::EPSILON) {
            return inter;
        }
        return Intersection();
    }

    const Intersection hitLeft = left->getIntersection(P, v);
    const Intersection hitRight = right->getIntersection(P, v);

    const bool leftHit = hitLeft.lambda >= Scene::EPSILON;
    const bool rightHit = hitRight.lambda >= Scene::EPSILON;

    if (leftHit && rightHit) {
        return (hitLeft.lambda < hitRight.lambda) ? hitLeft : hitRight;
    } else if (leftHit) {
        return hitLeft;
    } else {
        return hitRight;
    }
}

BoundingBox BVHNode::computeBoundingBox(const std::vector<std::shared_ptr<Shape>>& shapes) {
    Vector3 min(std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::infinity());

    Vector3 max(-std::numeric_limits<double>::infinity(),
                -std::numeric_limits<double>::infinity(),
                -std::numeric_limits<double>::infinity());

    for (const auto& shape : shapes) {
        shape->setBoundingBox();
        const std::shared_ptr<BoundingBox> b = shape->getBoundingBox();
        min = min.min(b->getMin());
        max = max.max(b->getMax());
    }

    return BoundingBox(min, max);
}
