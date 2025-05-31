#include "OBJ.h"
#include "scenes/Scene.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

OBJ::OBJ(const std::string& objFileName, const Vector3& position) : bvh(nullptr) {
    triangles = {};
    std::vector<Vector3> textures;
    std::vector<Vector3> vertexList;
    std::vector<Vector3> normalList;

    std::ifstream file(objFileName);
    if (!file.is_open()) {
        throw std::runtime_error("File not found: " + objFileName);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            vertexList.push_back(position + Vector3(x, y, z));

        } else if (prefix == "vn") {
            float x, y, z;
            iss >> x >> y >> z;
            normalList.emplace_back(x, y, z);

        } else if (prefix == "vt") {
            float u, v;
            iss >> u >> v;
            textures.emplace_back(u, 1.0f - v, 0.0f); // Inversion verticale

        } else if (prefix == "f") {
            std::vector<Vector3> faceVertices;
            std::vector<Vector3> faceUVs;
            std::string token;
            while (iss >> token) {
                std::istringstream tokenStream(token);
                std::string part;
                std::vector<std::string> parts;
                while (std::getline(tokenStream, part, '/')) {
                    parts.push_back(part);
                }

                int vertexIndex = std::stoi(parts[0]) - 1;
                faceVertices.push_back(vertexList[vertexIndex]);

                if (parts.size() > 1 && !parts[1].empty()) {
                    int textureIndex = std::stoi(parts[1]) - 1;
                    faceUVs.push_back(textures[textureIndex]);
                } else {
                    faceUVs.emplace_back(0, 0, 0);
                }
            }

            for (size_t i = 2; i < faceVertices.size(); ++i) {
                Vector3 A = faceVertices[0];
                Vector3 B = faceVertices[i - 1];
                Vector3 C = faceVertices[i];

                Vector3 uvA = faceUVs[0];
                Vector3 uvB = faceUVs[i - 1];
                Vector3 uvC = faceUVs[i];

                Triangle triangle(A, B, C);
                triangle.setTextureCoordinates(uvA, uvB, uvC);

                triangles.push_back(triangle);
            }
        }
    }

    this->rebuildBVH();
    this->update();
}

Intersection OBJ::getIntersection(const Vector3& P, const Vector3& v) const {
    if (!visible) return Intersection();

    const Intersection boxHit = bvh->getIntersection(P, v);
    if (boxHit.lambda < Scene::EPSILON) {
        return Intersection(); // No intersection
    }

    return boxHit;
}

// void OBJ::scale(double scaleFactor) {
//     Vector3 center = calculateCenter();
//
//     for (auto& triangle : triangles) {
//         triangle.A = center + (triangle.A - center) * scaleFactor;
//         triangle.B = center + (triangle.B - center) * scaleFactor;
//         triangle.C = center + (triangle.C - center) * scaleFactor;
//
//         triangle.normal = (triangle.B - triangle.A).cross(triangle.C - triangle.A).normalize();
//         triangle.distance = -triangle.normal.dot(triangle.A);
//
//         triangle.setBoundingBox();
//     }
//
//     delete bvh;
//     bvh = new BVHNode(triangles);
// }
//
// void OBJ::rotate(double angle, const Vector3& axis) {
//     Vector3 center = calculateCenter();
//
//     for (auto& triangle : triangles) {
//         triangle.A = center + (triangle.A - center).rotate(angle, axis);
//         triangle.B = center + (triangle.B - center).rotate(angle, axis);
//         triangle.C = center + (triangle.C - center).rotate(angle, axis);
//
//         triangle.normal = (triangle.B - triangle.A).cross(triangle.C - triangle.A).normalize();
//         triangle.distance = -triangle.normal.dot(triangle.A);
//
//         triangle.setBoundingBox();
//     }
//
//     delete bvh;
//     bvh = new BVHNode(triangles);
// }

Vector3 OBJ::calculateCenter() const
{
    Vector3 center(0, 0, 0);
    for (const auto& triangle : triangles) {
        for (const Vector3& vertex : triangle.getVertices())
        {
            center += vertex;
        }
    }
    return center * (1.0 / (triangles.size() * 3));
}

void OBJ::setBoundingBox() {
    Vector3 min(-std::numeric_limits<double>::max(),
                -std::numeric_limits<double>::max(),
                -std::numeric_limits<double>::max());

    Vector3 max(std::numeric_limits<double>::max(),
                std::numeric_limits<double>::max(),
                std::numeric_limits<double>::max());

    for (auto& triangle : triangles) {
        triangle.setBoundingBox();
        BoundingBox bb = *triangle.getBoundingBox();
        min = min.min(bb.getMin());
        max = max.max(bb.getMax());
    }

    this->boundingBox = std::make_unique<BoundingBox>(min, max);

    this->rebuildBVH();
}

void OBJ::update() {
    for (auto& triangle : triangles) {
        triangle.setColor(this->getColor());
        triangle.getMaterial().setShininess(this->getMaterial().getShininess());
        triangle.getMaterial().setEta(this->getMaterial().getEta());
        triangle.getMaterial().setReflectivity(this->getMaterial().getReflectivity());
        triangle.getMaterial().setTransparency(this->getMaterial().getTransparency());
        // triangle.getMaterial().setTexture(texture);
    }
}

Vector2 OBJ::getTextureCoordinates(const Vector3&) const {
    throw std::runtime_error("TextureCoordinates should be handled by inner Triangle");
}

void OBJ::rebuildBVH() {
    delete bvh;
    std::vector<std::shared_ptr<Shape>> triangleShapes;
    for (const auto& t : triangles) {
        triangleShapes.push_back(std::make_shared<Triangle>(t));
    }
    bvh = new BVHNode(triangleShapes);
}
