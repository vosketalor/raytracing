#include "Plane.h"

#include "BoundingBox.h"
#include "../scenes/Scene.h"

Plane::Plane(const Vector3& n, const double d)
    : normal(n.normalized()), distance(d) {
    setBoundingBox();
}

Plane::Plane(const Vector3& P, const Vector3& n) 
    : normal(n.normalized()), distance(-normal.dot(P)) {
    setBoundingBox();
}

Plane::Plane(const Vector3& C, const Vector3& v1, const Vector3& v2)
    : Plane(C, v1.cross(v2)) {}

Intersection Plane::getIntersection(const Vector3& P, const Vector3& v) const {
    if (!visible) return Intersection();

    const double denominator = normal.dot(v);

    if (std::abs(denominator) < Scene::EPSILON) {
        return Intersection();
    }

    const double numerator = normal.dot(P) + distance;
    const double lambda = -numerator / denominator;

    return (lambda >= 0) ? Intersection(lambda, normal, this) : Intersection();
}

void Plane::setBoundingBox() {
    const Vector3 min(-std::numeric_limits<double>::max(),
                -std::numeric_limits<double>::max(),
                -std::numeric_limits<double>::max());

    const Vector3 max(std::numeric_limits<double>::max(),
                std::numeric_limits<double>::max(),
                std::numeric_limits<double>::max());

    this->boundingBox = std::make_shared<BoundingBox>(min, max);
}


// void Plane::scale(double /*scale*/) {
//     // Scaling has no effect on an infinite plane
// }
//
// void Plane::rotate(double angle, const Vector3& axis) {
//     normal = normal.rotated(angle, axis).normalized();
// }
//
// void Plane::setBoundingBox() {
//     Vector3 min = {
//         -std::numeric_limits<double>::max(),
//         -std::numeric_limits<double>::max(),
//         -std::numeric_limits<double>::max()
//     };
//     Vector3 max = {
//         std::numeric_limits<double>::max(),
//         std::numeric_limits<double>::max(),
//         std::numeric_limits<double>::max()
//     };
//     boundingBox = std::make_shared<BoundingBox>(min, max);
// }

Vector2 Plane::getTextureCoordinates(const Vector3& intersection) const {
    Vector3 uAxis;

    if (std::abs(normal[0]) > 0.9) {
        uAxis = Vector3(0, 1, 0); // avoid colinearity
    } else {
        uAxis = Vector3(1, 0, 0);
    }

    uAxis = normal.cross(uAxis).normalized();
    const Vector3 vAxis = normal.cross(uAxis).normalized();

    const Vector3 origin = normal * (-distance);
    const Vector3 relative = intersection - origin;

    double u = relative.dot(uAxis);
    double v = relative.dot(vAxis);

    u = u - std::floor(u); // wrap u into [0,1)
    v = v - std::floor(v);

    return Vector2(u, v);
}