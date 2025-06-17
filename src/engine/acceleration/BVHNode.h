#pragma once

#include <vector>
#include <memory>
#include <set>
#include <queue> // Ajout pour BFS
#include "BoundingBox.h"
#include "GPUBVHNode.h"
#include "../shapes/Shape.h"

class BVHNode {
public:
    explicit BVHNode(const std::vector<std::shared_ptr<Shape>>& shapes, int depth = 0);

    std::vector<GPU::GPUBVHNode> toGPU(const std::vector<std::shared_ptr<Shape>>& allShapes) const;

    std::shared_ptr<BoundingBox> boundingBox;
    std::shared_ptr<BVHNode> left = nullptr;
    std::shared_ptr<BVHNode> right = nullptr;
    std::vector<std::shared_ptr<Shape>> leafShapes; // Utilisez seulement ceci

private:
    float computePartialArea(const std::vector<std::shared_ptr<Shape>>& shapes,
                             const std::vector<std::pair<float, size_t>>& sortedCenters,
                             size_t start, size_t end, int axis);

    BoundingBox computeBoundingBox(const std::vector<std::shared_ptr<Shape>>& shapes);
    int findShapeIndex(const std::shared_ptr<Shape>& shape,
                      const std::vector<std::shared_ptr<Shape>>& allShapes) const;
    int countNodes() const;

    bool isLeaf() const { return !leafShapes.empty(); }

public:
    int countAllShapes() const;
    void collectAllShapes(std::vector<std::shared_ptr<Shape>>& allShapes) const;
    int getMaxDepth() const;
    bool verifyIntegrity() const;
};

inline std::ostream& printBVH(std::ostream& os, const BVHNode& bvh, int depth = 0)
{
    std::string indent(depth * 2, ' '); // 2 espaces par niveau

    // CORRECTION : Teste leafShapes au lieu de leafShape
    if (!bvh.leafShapes.empty())
    {
        os << indent << "Leaf with " << bvh.leafShapes.size() << " shapes\n";
        for (size_t i = 0; i < bvh.leafShapes.size(); ++i) {
            os << indent << "  Shape " << i << ": " << bvh.leafShapes[i].get() << "\n";
        }
    }
    else
    {
        os << indent << "Internal Node - BoundingBox: " << bvh.boundingBox.get() << "\n";
    }

    if (bvh.left)
    {
        os << indent << "Left:\n";
        printBVH(os, *bvh.left, depth + 1);
    }

    if (bvh.right)
    {
        os << indent << "Right:\n";
        printBVH(os, *bvh.right, depth + 1);
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const BVHNode& bvh)
{
    return printBVH(os, bvh);
}