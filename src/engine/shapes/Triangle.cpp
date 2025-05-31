#include "Triangle.h"
#include <algorithm> // for std::min/std::max

#include "acceleration/BoundingBox.h"
#include "scenes/Scene.h"

Triangle::Triangle(const Vector3& A, const Vector3& B, const Vector3& C)
    : Plane(A, B - A, C - A), A(A), B(B), C(C), hasUV(false)
{
    Triangle::setBoundingBox();
}

std::vector<Vector3> Triangle::getVertices() const {
    return {A, B, C};
}

Intersection Triangle::getIntersection(const Vector3& P, const Vector3& v) const {
    if (!visible) return Intersection();

    // Appel à la méthode de la classe parente Plane
    const Intersection inter = Plane::getIntersection(P, v);
    if (inter.lambda < Scene::EPSILON) return Intersection();

    const Vector3 I = P + v * inter.lambda;
    return isInside(I) ? inter : Intersection();
}


bool Triangle::isInside(const Vector3& M) const {
    const Vector3 MA = A - M;
    const Vector3 MB = B - M;
    const Vector3 MC = C - M;

    const double norme2 = normal.norm() * normal.norm();

    const double alpha = normal.dot(MB.cross(MC)) / norme2;
    const double beta  = normal.dot(MC.cross(MA)) / norme2;
    const double gamma = normal.dot(MA.cross(MB)) / norme2;

    return (alpha >= 0) && (beta >= 0) && (gamma >= 0);
}

// void Triangle::scale(double scale) {
//     Vector3 center = (A + B + C) * (1.0 / 3.0);
//     A = center + (A - center) * scale;
//     B = center + (B - center) * scale;
//     C = center + (C - center) * scale;
//
//     Vector3 ab = B - A;
//     Vector3 ac = C - A;
//     normal = ab.cross(ac).normalize();
//     distance = -normal.dot(A);
//
//     Plane::scale(scale);
// }
//
// void Triangle::rotate(double angle, const Vector3& axis) {
//     Vector3 center = (A + B + C) * (1.0 / 3.0);
//     A = center + (A - center).rotate(angle, axis);
//     B = center + (B - center).rotate(angle, axis);
//     C = center + (C - center).rotate(angle, axis);
//
//     Vector3 ab = B - A;
//     Vector3 ac = C - A;
//     normal = ab.cross(ac).normalize();
//     distance = -normal.dot(A);
//
//     Plane::rotate(angle, axis);
// }

void Triangle::setBoundingBox() {
    const Vector3 minV = A.min(B).min(C);
    const Vector3 maxV = A.max(B).max(C);
    boundingBox = std::make_shared<BoundingBox>(minV, maxV);
}

Vector2 Triangle::getTextureCoordinates(const Vector3& intersection) const {
    if (!hasUV) {
        const Vector3 AB = B - A;
        const Vector3 AC = C - A;
        const Vector3 AP = intersection - A;

        const double u = AB.dot(AP) / AB.dot(AB);
        const double v = AC.dot(AP) / AC.dot(AC);

        return Vector2(u, v);
    } else {
        const Vector3 AB = B - A;
        const Vector3 AC = C - A;
        const Vector3 AP = intersection - A;

        const double d00 = AB.dot(AB);
        const double d01 = AB.dot(AC);
        const double d11 = AC.dot(AC);
        const double d20 = AP.dot(AB);
        const double d21 = AP.dot(AC);

        const double denom = d00 * d11 - d01 * d01;
        const double v = (d11 * d20 - d01 * d21) / denom;
        const double w = (d00 * d21 - d01 * d20) / denom;
        const double u = 1.0 - v - w;

        const Vector3 uv = uvA * u + uvB * v + uvC * w;

        return Vector2(uv[0], uv[1]);
    }
}

void Triangle::setTextureCoordinates(const Vector3& a, const Vector3& b, const Vector3& c) {
    uvA = a;
    uvB = b;
    uvC = c;
    hasUV = true;
}
