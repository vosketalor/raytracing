#include "Renderer.h"
#include "Renderer.h"
#include <cstdlib>

#include "LightSource.h"
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
    const Intersection result = findNearestIntersection(P, v);
    if (!result || result.shape == nullptr) return this->scene->getSkyColor();

    Vector3 intersectionPoint = P + v * result.lambda;
    Vector3 normal = result.normal;

    Vector3 color = result.shape->getColor();

    color *= this->scene->getAmbient();

    color += computeLighting(P, v, intersectionPoint, normal, *result.shape);

    return color;
}

Intersection Renderer::findNearestIntersection(const Vector3& P, const Vector3& v) const
{
    //TODO : BVH
    Intersection nearestIntersection;
    nearestIntersection.shape = nullptr;
    nearestIntersection.lambda = std::numeric_limits<double>::max();

    for (int i = 0; i < scene->getShapes().size(); ++i)
    {
        const Shape* shape = scene->getShapes()[i].get();
        if (!shape->isVisible()) continue;

        Intersection intersection = shape->getIntersection(P, v);
        if (intersection.shape != nullptr && intersection.lambda < nearestIntersection.lambda)
        {
            nearestIntersection = intersection;
        }
    }
    if (nearestIntersection.shape == nullptr) {
        return Intersection();
    }
    return nearestIntersection;

}

Vector3 Renderer::computeLighting(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape) const
{
    Vector3 color = Vector3(0, 0, 0);
    for (int i = 0; i < scene->getLightSources().size(); ++i)
    {
        const LightSource& lightSource = *scene->getLightSources()[i];
        Vector3 lightVec = lightSource.getPosition() - intersectionPoint;
        Vector3 lightDir = lightVec.normalized();
        Vector3 shadowOrigin = intersectionPoint + lightDir * Scene::EPSILON;

        Vector3 toLight = lightSource.getPosition() - shadowOrigin;
        const double lightDistance = toLight.norm();
        Vector3 shadowRayDir = toLight.normalized();

        if (!isInShadow(shadowOrigin, shadowRayDir, lightDistance))
        {
            const Vector3 diffuse = computeDiffuse(intersectionPoint, normal, shape, lightSource, shadowOrigin, shadowRayDir);
            const Vector3 specular = computeSpecular(P, v, intersectionPoint, normal, shape, lightSource);
            const double attenuation = computeAttenuation(lightDistance);
            color += (diffuse + specular) * attenuation;
        }
    }
    return color;
}


bool Renderer::isInShadow(const Vector3& shadowOrigin, const Vector3& shadowRayDir, const double lightDistance) const
{
    for (int i = 0; i < scene->getShapes().size(); ++i)
    {
        const Shape* shape = scene->getShapes()[i].get();
        if (!shape->isVisible()) continue;

        Intersection inter = shape->getIntersection(shadowOrigin, shadowRayDir);
        if (inter.shape != nullptr && inter.lambda < lightDistance)
        {
            return true;
        }
    }
    return false;
}


Vector3 Renderer::computeDiffuse(const Vector3& intersectionPoint, const Vector3& normal, const Shape& shape, const LightSource& lightSource, const Vector3& shadowOrigin, const Vector3& shadowRayDir) const
{
    //TODO : textures
    const double diffuseFactor = std::max(0.0, normal.dot(shadowRayDir));
    return shape.getColor() * lightSource.getColorDiffuse() * diffuseFactor;
}

Vector3 Renderer::computeSpecular(const Vector3& P, const Vector3& v, const Vector3& intersectionPoint,
    const Vector3& normal, const Shape& shape, const LightSource& lightSource) const
{
    const Vector3 viewDir = (P - intersectionPoint).normalized();
    const Vector3 lightDir = (lightSource.getPosition() - intersectionPoint).normalized();
    const Vector3 halfVector = (viewDir + lightDir).normalized();
    const double spec = std::max(0.0, halfVector.dot(normal));

    return lightSource.getColorSpecular() * lightSource.getIntensity() * std::pow(spec, shape.getShininess());
}

double Renderer::computeAttenuation(const double& distance) const
{
    Vector3 quadraticAttenuation = scene->getQuadraticAttenuation();
    return 1 / (quadraticAttenuation[0] + quadraticAttenuation[1] * distance + quadraticAttenuation[2] * distance * distance);
}




