#include "Renderer.h"
#include <cstdlib>

#include "shapes/Shape.h"

void Renderer::render(const int width, const int height, std::vector<Vector3>& frameBuffer) {
    // Paramètres de la caméra
    const Vector3 observer = {0, 0, 0};  // Position de la caméra
    const double distance = 1.0;         // Distance focale

    for (int ye = 0; ye < height; ++ye) {
        for (int xe = 0; xe < width; ++xe) {
            const double x = (width/2.0 - xe) / height;
            const double y = (height/2.0 - ye) / height;

            Vector3 rayDir = Vector3(x,y, -distance).normalized();

            frameBuffer[ye * width + xe] = getPixelColor(observer, rayDir);
        }
    }
}

Vector3 Renderer::getPixelColor(const Vector3& P, const Vector3& v)
{
    const Intersection intersection = findNearestIntersection(P, v);
    if (!intersection) return this->scene->getSkyColor();

    return intersection.shape->getColor();
}

Intersection Renderer::findNearestIntersection(const Vector3& P, const Vector3& v) const
{
    Intersection nearestIntersection;

    for (const auto& shape : scene->getShapes())
    {
        if (!shape->isVisible()) continue;

        const auto intersection = shape->getIntersection(P, v);

        if (intersection && (!nearestIntersection || intersection.lambda < nearestIntersection.lambda))
        {
            nearestIntersection = intersection;
        }
    }

    return nearestIntersection;
}

