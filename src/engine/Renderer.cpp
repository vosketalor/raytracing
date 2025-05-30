#include "Renderer.h"
#include "LightSource.h"
#include "shapes/Shape.h"

#include <cstdlib>
#include <execution>

#include <corecrt_math_defines.h>

static thread_local std::random_device rd;
static thread_local std::mt19937 gen(rd());
static thread_local std::uniform_real_distribution<double> dist(-1.0, 1.0);

#include "Camera.h"

void Renderer::render(const int width, const int height, std::vector<Vector3> &frameBuffer, const Camera camera)
{
    const Vector3 observer = camera.getPosition();
    const double aspectRatio = static_cast<double>(width) / height;

    // Calcul de la taille de l'écran virtuel en fonction du FOV (en radians)
    const double fovRad = camera.getFov() * M_PI / 180.0;
    const double screenHeight = 2.0 * tan(fovRad / 2.0);
    const double screenWidth = screenHeight * aspectRatio;

    const int blockSize = 32; // Taille des blocs

    // Découpage en blocs pour le parallélisme
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
        [&](const std::pair<int, int>& block)
        {
            const int bx = block.first;
            const int by = block.second;
            const int maxY = std::min(by + blockSize, height);
            const int maxX = std::min(bx + blockSize, width);

            for (int y = by; y < maxY; ++y)
            {
                for (int x = bx; x < maxX; ++x)
                {
                    // Normalisation des coordonnées écran entre -0.5 et 0.5
                    const double u = (x + 0.5) / width - 0.5;
                    const double v = 0.5 - (y + 0.5) / height;

                    // Passage aux dimensions physiques de l'écran virtuel (screen plane)
                    const double px = u * screenWidth;
                    const double py = v * screenHeight;

                    // Calcul des vecteurs de base de la caméra
                    Vector3 forward = camera.getDirection();       // direction caméra
                    Vector3 right = camera.getRight();              // vecteur droit
                    Vector3 up = camera.getUp();                     // vecteur haut

                    // Calcul du vecteur direction du rayon dans l'espace monde
                    Vector3 rayDir = (forward + px * right + py * up).normalized();

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

    const Vector3 intersectionPoint = P + v * result.lambda;
    const Vector3 normal = result.normal;

    Vector3 color = result.shape->getColor();

    color *= this->scene->getAmbient();

    color += computeLighting(P, v, intersectionPoint, normal, *result.shape);
    color += computeReflection(P, v, intersectionPoint, normal, *result.shape, order);
    color += computeRefraction(P, v, intersectionPoint, normal, *result.shape, order);
    return color;
}

Intersection Renderer::findNearestIntersection(const Vector3 &P, const Vector3 &v) const
{
    return this->bvh_.getIntersection(P, v);
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
    const Intersection intersection = this->bvh_.getIntersection(shadowOrigin, shadowRayDir);
    return intersection.lambda > Scene::EPSILON
        && intersection.shape != nullptr
        && intersection.lambda < lightDistance;
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
    return 1 / (quadraticAttenuation.x() + quadraticAttenuation.y() * distance + quadraticAttenuation.z() * distance * distance);
}

Vector3 Renderer::perturbVector(const Vector3 &direction, const Vector3 &normal, double roughness) const
{
    if (roughness <= 0.0) return direction;

    Vector3 tangent1;
    if (std::abs(normal.x()) < 0.9) {
        tangent1 = Vector3(0, normal.z(), -normal.y()).normalized();
    } else {
        tangent1 = Vector3(-normal.z(), 0, normal.x()).normalized();
    }
    const Vector3 tangent2 = normal.cross(tangent1).normalized();

    const double angle = dist(gen) * M_PI * 2.0;
    const double radius = dist(gen) * roughness;

    const Vector3 perturbation = tangent1 * (radius * std::cos(angle)) +
                          tangent2 * (radius * std::sin(angle));

    return (direction + perturbation).normalized();
}

Vector3 Renderer::computeReflection(const Vector3 &P, const Vector3 &v, const Vector3 &intersectionPoint,
                                    const Vector3 &normal, const Shape &shape, const int &order) const
{
    if (shape.getMaterial().getReflectivity() <= 0 || order <= 0)
        return Vector3(0, 0, 0);

    Vector3 reflectDir = (v - normal * 2 * normal.dot(v)).normalized();

    const double roughness = shape.getMaterial().getRoughness();
    if (roughness > 0.0) {
        reflectDir = perturbVector(reflectDir, normal, roughness);

        if (reflectDir.dot(normal) < 0) {
            reflectDir = reflectDir - normal * (2 * reflectDir.dot(normal));
        }
    }

    return getPixelColor(intersectionPoint, reflectDir, order - 1) * shape.getMaterial().getReflectivity();
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
