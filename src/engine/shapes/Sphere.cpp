#include "Sphere.h"

Sphere::Sphere(const Vector3& P, float radius) 
    : center(P), radius(radius) {
    // setBoundingBox();
}

Intersection Sphere::getIntersection(const Vector3& P, const Vector3& v) const {
    if (!visible) return Intersection();

    const Vector3 PC = P - center;
    const double a = v.dot(v);
    const double b = 2.0 * PC.dot(v);
    const double c = PC.dot(PC) - radius * radius;

    const double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0) return Intersection();

    const double sqrtDiscriminant = std::sqrt(discriminant);
    const double lambda1 = (-b - sqrtDiscriminant) / (2.0 * a);
    Vector3 I = P + v * lambda1;
    if (lambda1 > 0) return Intersection(lambda1, getNormal(I), this);

    const double lambda2 = (-b + sqrtDiscriminant) / (2.0 * a);
    I = P + v * lambda2;
    if (lambda2 > 0) return Intersection(lambda2, getNormal(I), this);

    return Intersection();
}

Vector3 Sphere::getNormal(const Vector3& P) const {
    return (P - center).normalized();
}

// void Sphere::scale(float scale) {
//     radius *= scale;
//     setBoundingBox();
// }
//
// void Sphere::rotate(float angle, const Vector3& axis) {
//     // Rotation has no effect on a sphere
// }

// void Sphere::setBoundingBox() {
//     Vector3 min = center - Vector3(radius, radius, radius);
//     Vector3 max = center + Vector3(radius, radius, radius);
//     boundingBox = std::make_shared<BoundingBox>(min, max);
// }

Vector2 Sphere::getTextureCoordinates(const Vector3& intersection) const {
    Vector3 P = (intersection - center).normalized();

    const double u = 0.5 + std::atan2(P[2], P[0]) / (2.0 * M_PI);
    const double v = 0.5 - std::asin(P[1]) / M_PI;

    return Vector2(u, v);
}