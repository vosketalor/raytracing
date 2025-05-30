#include "BoundingBox.h"

BoundingBox::BoundingBox(const Vector3& min, const Vector3& max)
    : min(min), max(max) {}

Vector3 BoundingBox::getMin() const {
    return min;
}

Vector3 BoundingBox::getMax() const {
    return max;
}

Intersection BoundingBox::getIntersection(const Vector3& P, const Vector3& v) const {
    double tmin = (min.x() - P.x()) / v.x();
    double tmax = (max.x() - P.x()) / v.x();

    if (tmin > tmax) std::swap(tmin, tmax);

    double tymin = (min.y() - P.y()) / v.y();
    double tymax = (max.y() - P.y()) / v.y();

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return Intersection();

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    double tzmin = (min.z() - P.z()) / v.z();
    double tzmax = (max.z() - P.z()) / v.z();

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return Intersection();

    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    return Intersection(tmin, Vector3(0, 0, 0), this);
}

void BoundingBox::setBoundingBox() {
    // No-op
}

Vector2 BoundingBox::getTextureCoordinates(const Vector3& /*intersection*/) const {
    return Vector2(0, 0);
}

bool BoundingBox::contains(const Vector3& P) const {
    return (P.x() >= min.x() && P.x() <= max.x()) &&
           (P.y() >= min.y() && P.y() <= max.y()) &&
           (P.z() >= min.z() && P.z() <= max.z());
}
