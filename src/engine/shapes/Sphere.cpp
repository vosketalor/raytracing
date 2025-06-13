#include "Sphere.h"

#include "../acceleration/BoundingBox.h"
//
// #ifdef _WIN32
//   #include <corecrt_math_defines.h>  // Windows (Visual Studio)
// #endif
//
Sphere::Sphere(const Vector3& P, const double radius)
    : center(P), radius(radius) {
    // setBoundingBox();
}

GPU::GPUShapeData Sphere::toGPU() const
{
    GPU::GPUShapeData data = Shape::toGPU();
    data.type = GPU::GPUShapeEnum::Sphere;
    data.center = glm::vec3(center.x(), center.y(), center.z());
    data.radius = static_cast<float>(radius);
    return data;
}

//
// Intersection Sphere::getIntersection(const Vector3& P, const Vector3& v) const {
//     if (!visible) return Intersection();
//
//     const Vector3 PC = P - center;
//     const double a = v.dot(v);
//     const double b = 2.0 * PC.dot(v);
//     const double c = PC.dot(PC) - radius * radius;
//
//     const double discriminant = b * b - 4.0 * a * c;
//     if (discriminant < 0) return Intersection();
//
//     const double sqrtDiscriminant = std::sqrt(discriminant);
//     const double lambda1 = (-b - sqrtDiscriminant) / (2.0 * a);
//     Vector3 I = P + v * lambda1;
//     if (lambda1 > 0) return Intersection(lambda1, getNormal(I), this);
//
//     const double lambda2 = (-b + sqrtDiscriminant) / (2.0 * a);
//     I = P + v * lambda2;
//     if (lambda2 > 0) return Intersection(lambda2, getNormal(I), this);
//
//     return Intersection();
// }
//
// Vector3 Sphere::getNormal(const Vector3& P) const {
//     return (P - center).normalized();
// }
//
// // void Sphere::scale(float scale) {
// //     radius *= scale;
// //     setBoundingBox();
// // }
// //
// // void Sphere::rotate(float angle, const Vector3& axis) {
// //     // Rotation has no effect on a sphere
// // }
//
// void Sphere::setBoundingBox() {
//     Vector3 min = center - Vector3(radius, radius, radius);
//     Vector3 max = center + Vector3(radius, radius, radius);
//     boundingBox = std::make_shared<BoundingBox>(min, max);
// }
//
// Vector2 Sphere::getTextureCoordinates(const Vector3& intersection) const {
//     Vector3 P = (intersection - center).normalized();
//
//     const double u = 0.5 + std::atan2(P[2], P[0]) / (2.0 * M_PI);
//     const double v = 0.5 - std::asin(P[1]) / M_PI;
//
//     return Vector2(u, v);
// }
//
// double Sphere::getDistanceNearestEdge(const Vector3& P, const Camera& camera) const
// {
//     // 1. Direction depuis la caméra vers le centre
//     const Vector3 toCenter = center - camera.getPosition();
//     const Vector3 viewDir = toCenter.normalized();
//
//     // 2. Vecteur du centre vers le point P
//     const Vector3 toPoint = P - center;
//
//     // 3. Projection de P sur le plan orthogonal à viewDir
//     const double distAlongView = toPoint.dot(viewDir);
//     const Vector3 projected = P - viewDir * distAlongView;
//
//     // 4. Rayon apparent dans ce plan
//     const double centerDistance = toCenter.norm();
//     if (centerDistance <= radius)
//         return 0.0; // Caméra à l'intérieur de la sphère : tout est "bord"
//
//     // Calcul du rayon apparent de la sphère dans le plan
//     const double sinTheta = radius / centerDistance;
//     const double apparentRadius = std::sqrt(toCenter.norm()*toCenter.norm() - radius * radius) * sinTheta;
//
//     // 5. Distance dans le plan
//     const double distanceInPlane = (projected - center).norm();
//
//     // 6. Distance au bord visible (disque)
//     return std::abs(distanceInPlane - apparentRadius);
// }
//
//
//
//
