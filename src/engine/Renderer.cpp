#include "Renderer.h"
#include "LightSource.h"
#include "shapes/Shape.h"

#include <cstdlib>
#include <execution>

void Renderer::render(const int width, const int height, std::vector<Vector3> &frameBuffer)
{
    const Vector3 observer = {0, 0, 0};
    const double distance = 1.0;

    const int blockSize = 32; // Taille de bloc (peut être ajustée selon tests)

    // Parcours des blocs en parallèle
    std::vector<std::pair<int, int>> blocks;
    for (int by = 0; by < height; by += blockSize)
    {
        for (int bx = 0; bx < width; bx += blockSize)
        {
            blocks.emplace_back(bx, by);
        }
    }

    std::for_each(
        std::execution::par,
        blocks.begin(),
        blocks.end(),
        [&](const std::pair<int, int> &block)
        {
            int bx = block.first;
            int by = block.second;
            int maxY = std::min(by + blockSize, height);
            int maxX = std::min(bx + blockSize, width);

            for (int y = by; y < maxY; ++y)
            {
                for (int x = bx; x < maxX; ++x)
                {
                    const double fx = (width / 2.0 - x) / height;
                    const double fy = (height / 2.0 - y) / height;
                    Vector3 rayDir = Vector3(fx, fy, -distance).normalized();
                    frameBuffer[y * width + x] = getPixelColor(observer, rayDir, 10);
                }
            }
        });
}

Vector3 Renderer::getPixelColor(const Vector3 &P, const Vector3 &v, const int &order) const
{
    const Intersection result = findNearestIntersection(P, v);
    if (!result || result.shape == nullptr)
        return this->scene->getSkyColor();

    Vector3 intersectionPoint = P + v * result.lambda;
    Vector3 normal = result.normal;

    Vector3 color = result.shape->getColor();

    color *= this->scene->getAmbient();

    color += computeLighting(P, v, intersectionPoint, normal, *result.shape);
    color += computeReflection(P, v, intersectionPoint, normal, *result.shape, order);
    color += computeRefraction(P, v, intersectionPoint, normal, *result.shape, order);
    return color;
}

Intersection Renderer::findNearestIntersection(const Vector3 &P, const Vector3 &v) const
{
    // TODO : BVH
    Intersection nearestIntersection;
    nearestIntersection.shape = nullptr;
    nearestIntersection.lambda = std::numeric_limits<double>::max();

    for (int i = 0; i < scene->getShapes().size(); ++i)
    {
        const Shape *shape = scene->getShapes()[i].get();
        if (!shape->isVisible())
            continue;

        Intersection intersection = shape->getIntersection(P, v);
        if (intersection.shape != nullptr && intersection.lambda < nearestIntersection.lambda && intersection.lambda > Scene::EPSILON)
        {
            nearestIntersection = intersection;
        }
    }
    if (nearestIntersection.shape == nullptr)
    {
        return Intersection();
    }
    return nearestIntersection;
}

Vector3 Renderer::computeLighting(const Vector3 &P, const Vector3 &v, const Vector3 &intersectionPoint, const Vector3 &normal, const Shape &shape) const
{
    Vector3 color = Vector3(0, 0, 0);
    for (int i = 0; i < scene->getLightSources().size(); ++i)
    {
        const LightSource &lightSource = *scene->getLightSources()[i];
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

bool Renderer::isInShadow(const Vector3 &shadowOrigin, const Vector3 &shadowRayDir, const double lightDistance) const
{
    for (int i = 0; i < scene->getShapes().size(); ++i)
    {
        const Shape *shape = scene->getShapes()[i].get();
        if (!shape->isVisible())
            continue;

        Intersection inter = shape->getIntersection(shadowOrigin, shadowRayDir);
        if (inter.shape != nullptr && inter.lambda < lightDistance)
        {
            return true;
        }
    }
    return false;
}

Vector3 Renderer::computeDiffuse(const Vector3 &intersectionPoint, const Vector3 &normal, const Shape &shape, const LightSource &lightSource, const Vector3 &shadowOrigin, const Vector3 &shadowRayDir) const
{
    // TODO : textures
    const double diffuseFactor = std::max(0.0, normal.dot(shadowRayDir));
    return shape.getColor() * lightSource.getColorDiffuse() * diffuseFactor;
}

Vector3 Renderer::computeSpecular(const Vector3 &P, const Vector3 &v, const Vector3 &intersectionPoint,
                                  const Vector3 &normal, const Shape &shape, const LightSource &lightSource) const
{
    const Vector3 viewDir = (P - intersectionPoint).normalized();
    const Vector3 lightDir = (lightSource.getPosition() - intersectionPoint).normalized();
    const Vector3 halfVector = (viewDir + lightDir).normalized();
    const double spec = std::max(0.0, halfVector.dot(normal));

    return lightSource.getColorSpecular() * lightSource.getIntensity() * std::pow(spec, shape.getMaterial().getShininess());
}

double Renderer::computeAttenuation(const double &distance) const
{
    Vector3 quadraticAttenuation = scene->getQuadraticAttenuation();
    return 1 / (quadraticAttenuation[0] + quadraticAttenuation[1] * distance + quadraticAttenuation[2] * distance * distance);
}

Vector3 Renderer::computeReflection(const Vector3 &P, const Vector3 &v, const Vector3 &intersectionPoint,
                                    const Vector3 &normal, const Shape &shape, const int &order) const
{
    if (shape.getMaterial().getReflectivity() <= 0 || order <= 0)
        return Vector3(0, 0, 0);

    const Vector reflectDir = (v - normal * 2 * normal.dot(v)).normalized();
    return getPixelColor(intersectionPoint, reflectDir, order-1) *  shape.getMaterial().getReflectivity();
}

Vector3 Renderer::computeRefraction(const Vector3 &P, const Vector3 &v, const Vector3 &intersectionPoint,
                                    const Vector3 &normal, const Shape &shape, const int &order) const
{
    if (shape.getMaterial().getTransparency() <= 0 || order <= 0)
        return Vector3(0, 0, 0);

    const Vector3 i = v.normalized();
    Vector3 n = normal.normalized();
    double c1 = normal.dot(i);
    const bool exiting = c1 >= 0;

    const double etaI = Scene::ETA_AIR;
    const double etaT = shape.getMaterial().getEta();
    double eta;

    if (exiting)
    {
        n = -n;
        eta = etaT / etaI;
    }
    else
    {
        c1 = -c1;
        eta = etaI / etaT;
    }

    const double sin2ThetaI = 1.0 - c1 * c1;
    const double sin2ThetaT = eta * eta * sin2ThetaI;
    const double k = 1.0 - sin2ThetaT;

    if (k >= 0)
    {
        const double c2 = std::sqrt(k);
        const Vector3 refractDir = (i * eta + normal * (eta * c1 - c2)).normalized();
        const Vector3 offset = refractDir * Scene::EPSILON;
        return getPixelColor(intersectionPoint + offset, refractDir, order - 1) * shape.getMaterial().getTransparency();
    }

    const Vector3 reflectDir = (i - normal * 2 * normal.dot(i)).normalized();
    const Vector3 offset = reflectDir * Scene::EPSILON;
    return getPixelColor(intersectionPoint + offset, reflectDir, order - 1) * shape.getMaterial().getTransparency();
}
