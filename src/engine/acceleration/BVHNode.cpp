#include "BVHNode.h"

#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>

BVHNode::BVHNode(const std::vector<std::shared_ptr<Shape>>& shapes, int depth)
{
    if (shapes.empty()) {
        std::cerr << "ERREUR: Tentative de création d'un nœud BVH avec 0 shapes!" << std::endl;
        return;
    }

    // Condition d'arrêt : UNE SEULE SHAPE = FEUILLE
    if (shapes.size() == 1) {
        leafShapes = shapes;
        boundingBox = std::make_shared<BoundingBox>(computeBoundingBox(shapes));

        shapes[0]->setBoundingBox();

        return;
    }

    // Calculer la bounding box de ce nœud
    boundingBox = std::make_shared<BoundingBox>(computeBoundingBox(shapes));

    // S'assurer que toutes les shapes ont leur bounding box
    for (const auto& shape : shapes) {
        shape->setBoundingBox();
    }

    // Trouver le meilleur axe de division (celui avec la plus grande étendue)
    int bestAxis = 0;
    float maxExtent = 0.0f;

    const glm::vec3 extent = boundingBox->getMax() - boundingBox->getMin();
    for (int axis = 0; axis < 3; ++axis) {
        if (extent[axis] > maxExtent) {
            maxExtent = extent[axis];
            bestAxis = axis;
        }
    }

    // Si toutes les shapes sont au même point, on divise quand même arbitrairement
    if (maxExtent < 1e-6f) {
        // Division arbitraire au milieu
        size_t splitPoint = shapes.size() / 2;
        if (splitPoint == 0) splitPoint = 1;

        std::vector<std::shared_ptr<Shape>> leftShapes(shapes.begin(), shapes.begin() + splitPoint);
        std::vector<std::shared_ptr<Shape>> rightShapes(shapes.begin() + splitPoint, shapes.end());

        left = std::make_shared<BVHNode>(leftShapes, depth + 1);
        right = std::make_shared<BVHNode>(rightShapes, depth + 1);
        return;
    }

    // Trier les shapes selon l'axe choisi
    auto sortedShapes = shapes;
    std::sort(sortedShapes.begin(), sortedShapes.end(),
        [bestAxis](const std::shared_ptr<Shape>& a, const std::shared_ptr<Shape>& b) {
            const auto& bboxA = a->getBoundingBox();
            const auto& bboxB = b->getBoundingBox();
            float centerA = (bboxA->getMin()[bestAxis] + bboxA->getMax()[bestAxis]) * 0.5f;
            float centerB = (bboxB->getMin()[bestAxis] + bboxB->getMax()[bestAxis]) * 0.5f;
            return centerA < centerB;
        });

    // Division au milieu
    size_t splitPoint = sortedShapes.size() / 2;

    // S'assurer qu'on ne crée pas de partitions vides
    if (splitPoint == 0) splitPoint = 1;
    if (splitPoint >= sortedShapes.size()) splitPoint = sortedShapes.size() - 1;

    std::vector<std::shared_ptr<Shape>> leftShapes(sortedShapes.begin(), sortedShapes.begin() + splitPoint);
    std::vector<std::shared_ptr<Shape>> rightShapes(sortedShapes.begin() + splitPoint, sortedShapes.end());

    // Vérification de sécurité - ne devrait jamais arriver avec la nouvelle logique
    if (leftShapes.empty() || rightShapes.empty()) {
        std::cerr << "ERREUR CRITIQUE: Division créant une partition vide!" << std::endl;
        std::cerr << "Shapes total: " << shapes.size() << ", splitPoint: " << splitPoint << std::endl;

        // Fallback d'urgence - créer des feuilles même avec plusieurs shapes
        if (leftShapes.empty()) {
            leafShapes = rightShapes;
            return;
        }
        if (rightShapes.empty()) {
            leafShapes = leftShapes;
            return;
        }
    }

    // Construction récursive - continue jusqu'à avoir 1 shape par feuille
    left = std::make_shared<BVHNode>(leftShapes, depth + 1);
    right = std::make_shared<BVHNode>(rightShapes, depth + 1);
}

float BVHNode::computePartialArea(const std::vector<std::shared_ptr<Shape>>& shapes,
                                 const std::vector<std::pair<float, size_t>>& sortedCenters,
                                 size_t start, size_t end, int axis) {
    if (start >= end) return 0.0f;

    glm::vec3 minBounds(std::numeric_limits<float>::max());
    glm::vec3 maxBounds(std::numeric_limits<float>::lowest());

    for (size_t i = start; i < end; ++i) {
        const auto& shape = shapes[sortedCenters[i].second];
        const auto& bbox = shape->getBoundingBox();
        minBounds = glm::min(minBounds, bbox->getMin());
        maxBounds = glm::max(maxBounds, bbox->getMax());
    }

    glm::vec3 extent = maxBounds - minBounds;
    return 2.0f * (extent.x * extent.y + extent.y * extent.z + extent.z * extent.x);
}

BoundingBox BVHNode::computeBoundingBox(const std::vector<std::shared_ptr<Shape>>& shapes) {
    if (shapes.empty()) {
        std::cerr << "ATTENTION: computeBoundingBox appelé avec 0 shapes!" << std::endl;
        return BoundingBox(glm::vec3(0.0f), glm::vec3(0.0f));
    }

    glm::vec3 min(std::numeric_limits<float>::max());
    glm::vec3 max(std::numeric_limits<float>::lowest());

    for (const auto& shape : shapes) {
        shape->setBoundingBox();
        const std::shared_ptr<BoundingBox> bbox = shape->getBoundingBox();
        if (bbox) {
            min = glm::min(min, bbox->getMin());
            max = glm::max(max, bbox->getMax());
        }
    }

    return BoundingBox(min, max);
}

// Fonction pour compter récursivement toutes les shapes dans le BVH
int BVHNode::countAllShapes() const {
    if (isLeaf()) {
        return static_cast<int>(leafShapes.size());
    }

    int count = 0;
    if (left) count += left->countAllShapes();
    if (right) count += right->countAllShapes();
    return count;
}

// Fonction pour collecter toutes les shapes du BVH
void BVHNode::collectAllShapes(std::vector<std::shared_ptr<Shape>>& allShapes) const {
    if (isLeaf()) {
        allShapes.insert(allShapes.end(), leafShapes.begin(), leafShapes.end());
        return;
    }

    if (left) left->collectAllShapes(allShapes);
    if (right) right->collectAllShapes(allShapes);
}

// Fonction pour calculer la profondeur maximale du BVH
int BVHNode::getMaxDepth() const {
    if (isLeaf()) {
        return 1;
    }

    int leftDepth = left ? left->getMaxDepth() : 0;
    int rightDepth = right ? right->getMaxDepth() : 0;

    return 1 + std::max(leftDepth, rightDepth);
}

// Fonction pour vérifier l'intégrité du BVH
bool BVHNode::verifyIntegrity() const {
    if (isLeaf()) {
        if (leafShapes.size() != 1) {
            std::cerr << "ERREUR: Feuille avec " << leafShapes.size() << " shapes au lieu de 1!" << std::endl;
            return false;
        }
        return true;
    }

    if (!left || !right) {
        std::cerr << "ERREUR: Nœud interne avec enfant manquant!" << std::endl;
        return false;
    }

    return left->verifyIntegrity() && right->verifyIntegrity();
}

std::vector<GPU::GPUBVHNode> BVHNode::toGPU(const std::vector<std::shared_ptr<Shape>>& allShapes) const {
    // Vérifications initiales
    int shapeCount = countAllShapes();
    int maxDepth = getMaxDepth();
    int totalNodes = countNodes();

    if (!verifyIntegrity()) {
        std::cerr << "ERROR: BVH integrity check failed!" << std::endl;
    }

    if (shapeCount != static_cast<int>(allShapes.size())) {
        std::cerr << "CRITICAL ERROR: Shape count mismatch!" << std::endl;
        // ... (debug code inchangé) ...
        return {};
    }

    std::vector<GPU::GPUBVHNode> gpuNodes(totalNodes);
    std::queue<std::pair<const BVHNode*, int>> nodeQueue;
    int nextIndex = 0;
    nodeQueue.push({this, nextIndex});

    while (!nodeQueue.empty()) {
        auto [currentNode, idx] = nodeQueue.front();
        nodeQueue.pop();

        GPU::GPUBVHNode& gpuNode = gpuNodes[idx];
        gpuNode.minBounds = currentNode->boundingBox->getMin();
        gpuNode.maxBounds = currentNode->boundingBox->getMax();

        if (currentNode->isLeaf()) {
            gpuNode.leftChild = -1;
            gpuNode.rightChild = -1;
            if (!currentNode->leafShapes.empty()) {
                gpuNode.shapeIndex = findShapeIndex(currentNode->leafShapes[0], allShapes);
            } else {
                gpuNode.shapeIndex = -1;
                std::cerr << "ERROR: Empty leaf node!" << std::endl;
            }
        } else {
            gpuNode.shapeIndex = -1;

            // Process left child
            if (currentNode->left) {
                nextIndex++;
                gpuNode.leftChild = nextIndex;
                nodeQueue.push({currentNode->left.get(), nextIndex});
            } else {
                gpuNode.leftChild = -1;
            }

            // Process right child
            if (currentNode->right) {
                nextIndex++;
                gpuNode.rightChild = nextIndex;
                nodeQueue.push({currentNode->right.get(), nextIndex});
            } else {
                gpuNode.rightChild = -1;
            }
        }
    }

    // Validation
    if (nextIndex + 1 != totalNodes) {
        std::cerr << "ERROR: Node count mismatch! Expected: " << totalNodes
                  << ", Actual: " << (nextIndex + 1) << std::endl;
    }
    return gpuNodes;
}

int BVHNode::findShapeIndex(const std::shared_ptr<Shape>& shape,
                           const std::vector<std::shared_ptr<Shape>>& allShapes) const {
    for (size_t i = 0; i < allShapes.size(); ++i) {
        if (allShapes[i] == shape) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int BVHNode::countNodes() const {
    int count = 1;
    if (left) count += left->countNodes();
    if (right) count += right->countNodes();
    return count;
}