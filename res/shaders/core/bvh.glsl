uniform int numBVHNodes;
uniform bool useBVH;

struct GPUBVHNode {
    vec3 minBounds;
    float pad1;
    vec3 maxBounds;
    int leftChild;      // Index du fils gauche (-1 si feuille)
    int rightChild;     // Index du fils droit (-1 si feuille)
    int shapeIndex;     // Index de la forme si feuille (-1 sinon)
    int pad2;
    int pad3;
};

layout(std430, binding = 4) buffer BVHData {
    GPUBVHNode bvhNodes[];
};

// Fonction d'intersection avec une bounding box
bool intersectAABB(Ray ray, vec3 minBounds, vec3 maxBounds, out float tNear, out float tFar) {
    vec3 invDir = 1.0 / ray.direction;
    vec3 t0 = (minBounds - ray.origin) * invDir;
    vec3 t1 = (maxBounds - ray.origin) * invDir;

    vec3 tMin = min(t0, t1);
    vec3 tMax = max(t0, t1);

    tNear = max(tMin.x, max(tMin.y, tMin.z));
    tFar = min(tMax.x, min(tMax.y, tMax.z));

    return tNear <= tFar && tFar > 0.0 && tNear < MAX_DIST;
}

HitInfo findNearestIntersectionBVH(Ray ray) {
    HitInfo hit;
    hit.hit = false;
    hit.t = MAX_DIST;

    if (numBVHNodes == 0) return hit;

    // Utilisation d'une pile fixe plus efficace
    int stack[24];
    int stackPtr = 0;
    stack[stackPtr++] = 0; // Racine

    while (stackPtr > 0) {
        int nodeIndex = stack[--stackPtr];
        GPUBVHNode node = bvhNodes[nodeIndex];

        // Test rapide de la bounding box
        float tNear, tFar;
        if (!intersectAABB(ray, node.minBounds, node.maxBounds, tNear, tFar) || tNear > hit.t) {
            continue;
        }

        // Feuille - test d'intersection
        if (node.shapeIndex >= 0) {
            if (node.shapeIndex < numShapes) {
                float t;
                GPUShape shape = shapes[node.shapeIndex];
                if (intersect(ray, shape, t) && t < hit.t && t > EPSILON) {
                    hit.hit = true;
                    hit.t = t;
                    hit.point = ray.origin + ray.direction * t;
                    hit.normal = getNormal(shape, hit.point);
                    hit.color = shape.color;
                    hit.shapeIndex = node.shapeIndex;
                }
            }
            continue;
        }

        // Récupération des enfants avec préchargement mémoire
        int leftChild = node.leftChild;
        int rightChild = node.rightChild;
        GPUBVHNode leftNode = bvhNodes[leftChild];
        GPUBVHNode rightNode = bvhNodes[rightChild];

        // Test des AABB enfants
        float leftNear, leftFar, rightNear, rightFar;
        bool hitLeft = intersectAABB(ray, leftNode.minBounds, leftNode.maxBounds, leftNear, leftFar);
        bool hitRight = intersectAABB(ray, rightNode.minBounds, rightNode.maxBounds, rightNear, rightFar);

        // Optimisation : ne pousser que les enfants potentiellement visibles
        if (hitLeft && hitRight) {
            // Pousser l'enfant le plus loin d'abord
            if (leftNear > rightNear) {
                stack[stackPtr++] = leftChild;
                stack[stackPtr++] = rightChild;
            } else {
                stack[stackPtr++] = rightChild;
                stack[stackPtr++] = leftChild;
            }
        }
        else if (hitLeft) {
            stack[stackPtr++] = leftChild;
        }
        else if (hitRight) {
            stack[stackPtr++] = rightChild;
        }
    }

    return hit;
}