#include "Renderer.h"
#include "LightSource.h"
#include "shapes/Shape.h"

#include <cstdlib>
#include <execution>

#ifdef _WIN32
  #include <corecrt_math_defines.h>  // Windows (Visual Studio)
#endif

#include "Camera.h"

void Renderer::render(std::vector<Vector3> &frameBuffer) const
{
    const Vector3 observer = this->camera_.getPosition();
    const double aspectRatio = static_cast<double>(width) / height;
    const double fovRad = this->camera_.getFov() * M_PI / 180.0;
    const double screenHeight = 2.0 * tan(fovRad / 2.0);
    const double screenWidth = screenHeight * aspectRatio;

    const int dofSamples = camera_.getDofSamples();
    const int w = width;
    const int h = height;

    Vector3* frameBuffer_ptr = frameBuffer.data();
    #pragma acc data copy(frameBuffer_ptr[0:w*h])
    {
        #pragma acc parallel loop independent collapse(2) default(present)
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                unsigned int seed = (y * w + x) * 2654435761u;
                auto rand_double = [&seed](const double min, const double max) -> double {
                    seed = (seed * 1664525u + 1013904223u);
                    const double normalized = (seed & 0xFFFFFF) / 16777216.0;
                    return min + normalized * (max - min);
                };

                const double u = (x + 0.5) / w - 0.5;
                const double v = 0.5 - (y + 0.5) / h;
                const double px = u * screenWidth;
                const double py = v * screenHeight;

                Vector3 forward = this->camera_.getDirection();
                Vector3 right = this->camera_.getRight();
                Vector3 up = this->camera_.getUp();

                Vector3 colorSum(0, 0, 0);

                for (int i = 0; i < dofSamples; ++i) {
                    Vector3 rayDir = (forward + px * right + py * up).normalized();
                    Vector3 focusPoint = observer + rayDir * camera_.getFocusDistance();

                    Vector3 offset2D;
                    while (true) {
                        double x_rand = rand_double(-1.0, 1.0);
                        double y_rand = rand_double(-1.0, 1.0);
                        if (x_rand*x_rand + y_rand*y_rand >= 1.0) continue;
                        offset2D = Vector3(x_rand, y_rand, 0);
                        break;
                    }

                    Vector3 offset = offset2D.x() * right + offset2D.y() * up;
                    Vector3 rayOrigin = observer + offset * (camera_.getAperture() * 0.5);
                    Vector3 newDir = (focusPoint - rayOrigin).normalized();

                    // Calcul de la couleur avec propagation du seed
                    colorSum += getPixelColor(rayOrigin, newDir, 10, &seed);
                }
                frameBuffer[y * w + x] = colorSum * (1.0 / static_cast<double>(dofSamples));
            }
        }
    }
}

const Shape* Renderer::getShape(const double& x, const double& y)
{
    const double aspectRatio = static_cast<double>(width) / height;

    // Calcul de la taille de l'écran virtuel en fonction du FOV
    const double fovRad = this->camera_.getFov() * M_PI / 180.0;
    const double screenHeight = 2.0 * tan(fovRad / 2.0);
    const double screenWidth = screenHeight * aspectRatio;

    // Normalisation des coordonnées écran entre -0.5 et 0.5
    const double u = (x + 0.5) / width - 0.5;
    const double v = 0.5 - (y + 0.5) / height;

    // Position sur le plan image
    const double px = u * screenWidth;
    const double py = v * screenHeight;

    // Vecteurs de base de la caméra
    const Vector3 forward = this->camera_.getDirection();
    const Vector3 right = this->camera_.getRight();
    const Vector3 up = this->camera_.getUp();

    // Direction du rayon
    const Vector3 rayDir = (forward + px * right + py * up).normalized();

    // Lancement du rayon
    const Vector3 origin = this->camera_.getPosition();
    const Intersection result = findNearestIntersection(origin, rayDir);

    // Renvoi de la forme touchée
    if (!result || result.shape == nullptr)
        return nullptr;

    return result.shape;
}

void Renderer::setCamera(const Camera& camera)
{
    this->camera_ = camera;
}


Vector3 Renderer::getPixelColor(const Vector3 &P, const Vector3 &v, const int &order, unsigned int* seed) const
{
    const Intersection result = findNearestIntersection(P, v);
    if (!result || result.shape == nullptr)
        return this->scene->getSkyColor();

    const Vector3 intersectionPoint = P + v * result.lambda;
    const Vector3 normal = result.normal;

    if (result.shape->isWireframeEnabled())
    {
        const double distance = result.shape->getDistanceNearestEdge(intersectionPoint, this->camera_);

        if (distance < Scene::WIREFRAME_THICKNESS)
            return Scene::WIREFRAME_COLOR;
    }

    Vector3 color = result.shape->getColor();

    color *= this->scene->getAmbient();

    color += computeLighting(P, v, intersectionPoint, normal, *result.shape, seed);
    color += computeReflection(P, v, intersectionPoint, normal, *result.shape, order, seed);
    color += computeRefraction(P, v, intersectionPoint, normal, *result.shape, order, seed);
    return color;
}

Intersection Renderer::findNearestIntersection(const Vector3 &P, const Vector3 &v) const
{
    return this->bvh_.getIntersection(P, v);
}

Vector3 Renderer::computeLighting(const Vector3 &P,
                                  const Vector3 &v,
                                  const Vector3 &intersectionPoint,
                                  const Vector3 &normal,
                                  const Shape &shape,
                                  unsigned int* seed) const
{
    Vector3 result(0, 0, 0);

    // Nombre d'échantillons pour l’area light (1 = point light pur)
    const int numSamples = 16;

    for (const auto& lightPtr : scene->getLightSources())
    {
        const LightSource &L = *lightPtr;
        Vector3 accumLight(0, 0, 0);

        // Pour chaque échantillon sur l’area light
        for (int i = 0; i < numSamples; ++i)
        {
            // 1) On prélève un point aléatoire sur l’area light
            Vector3 samplePos = L.samplePointOnArea();
            Vector3 Lvec     = samplePos - intersectionPoint;
            double  Ldist    = Lvec.norm();
            Vector3 Ldir     = Lvec * (1 / Ldist);
            Vector3 shadowOrig = intersectionPoint + Ldir * Scene::EPSILON;

            // 2) On calcule l’atténuation d’ombre colorée
            Vector3 attenShadow = computeShadowAttenuation(shadowOrig, Ldir, Ldist);
            if (attenShadow.x() <= 0.0 && attenShadow.y() <= 0.0 && attenShadow.z() <= 0.0)
                continue; // entièrement bloqué

            // 3) Diffuse
            double NdotL = std::max(0.0, normal.dot(Ldir));
            Vector3 diffuse = shape.getColor() * L.getColorDiffuse() * NdotL;

            // 4) Spéculaire (on recalculera la demi‐vector pour ce Ldir)
            Vector3 viewDir = (P - intersectionPoint).normalized();
            Vector3 halfVec = (viewDir + Ldir).normalized();
            double specAngle = std::max(0.0, normal.dot(halfVec));
            Vector3 specular = L.getColorSpecular()
                             * L.getIntensity()
                             * std::pow(specAngle, shape.getMaterial().getShininess());

            // 5) Atténuation par distance
            double attenDist = computeAttenuation(Ldist);

            // 6) On cumule ce que voit ce sample
            accumLight += (diffuse + specular) * attenDist * attenShadow;
        }

        // 7) Moyenne sur tous les échantillons
        result += accumLight * (1.0 / (double)numSamples);
    }

    return result;
}

bool Renderer::isInShadow(const Vector3 &shadowOrigin, const Vector3 &shadowRayDir, const double lightDistance) const
{
    const Intersection intersection = this->bvh_.getIntersection(shadowOrigin, shadowRayDir);
    return intersection.lambda > Scene::EPSILON
        && intersection.shape != nullptr
        && intersection.lambda < lightDistance;
}

Vector3 Renderer::computeShadowAttenuation(const Vector3& origin, const Vector3& dir, double lightDist) const
{
    Vector3 attenuation(1, 1, 1);
    Vector3 currOrig = origin;
    Vector3 currDir = dir;

    while (true)
    {
        Intersection hit = this->bvh_.getIntersection(currOrig, currDir);
        // si pas d'intersection ou intersection au-delà de la lumière, on s'arrête
        if (!hit || hit.lambda >= lightDist)
            break;

        const Shape* sh = hit.shape;
        const Material& m = sh->getMaterial();
        double t = m.getTransparency();         // [0,1]
        Vector3 col = sh->getColor();           // couleur de l’objet
        // on filtre l’atténuation : la lumière est multipliée par t * col
        attenuation = attenuation * (t * col);
        // si l’objet est totalement opaque, on coupe tout
        if (t <= 0.0)
            return {0, 0, 0};

        // avancer l’origine juste après l’intersection
        Vector3 hitPt = currOrig + currDir * hit.lambda;
        currOrig = hitPt + currDir * Scene::EPSILON;

        // réduire la distance restante jusqu’à la lumière
        lightDist -= hit.lambda + Scene::EPSILON;
        if (attenuation.x() <= 1e-3 && attenuation.y() <= 1e-3 && attenuation.z() <= 1e-3)
            return {0, 0, 0};
    }

    return attenuation;
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

Vector3 Renderer::perturbVector(const Vector3 &direction,
                               const Vector3 &normal,
                               double roughness,
                               unsigned int* seed) const
{
    auto rand_double = [&seed](const double min, const double max) -> double {
        *seed = (*seed * 1664525u + 1013904223u);
        const double normalized = (*seed & 0xFFFFFF) / 16777216.0;
        return min + normalized * (max - min);
    };

    if (roughness <= 0.0) return direction;

    Vector3 tangent1;
    if (std::abs(normal.x()) < 0.9) {
        tangent1 = Vector3(0, normal.z(), -normal.y()).normalized();
    } else {
        tangent1 = Vector3(-normal.z(), 0, normal.x()).normalized();
    }
    Vector3 tangent2 = normal.cross(tangent1).normalized();

    double angle = rand_double(0.0, M_PI * 2.0);
    double r = rand_double(0.0, roughness);

    Vector3 perturbation = tangent1 * (r * cos(angle)) +
                           tangent2 * (r * sin(angle));

    return (direction + perturbation).normalized();
}

Vector3 Renderer::computeReflection(const Vector3 &P, const Vector3 &v, const Vector3 &intersectionPoint,
                                    const Vector3 &normal, const Shape &shape, const int &order, unsigned int* seed) const
{
    if (shape.getMaterial().getReflectivity() <= 0 || order <= 0)
        return Vector3(0, 0, 0);

    Vector3 reflectDir = (v - normal * 2 * normal.dot(v)).normalized();

    const double roughness = shape.getMaterial().getRoughness();
    if (roughness > 0.0) {
        reflectDir = perturbVector(reflectDir, normal, roughness, seed);

        if (reflectDir.dot(normal) < 0) {
            reflectDir = reflectDir - normal * (2 * reflectDir.dot(normal));
        }
    }

    return getPixelColor(intersectionPoint, reflectDir, order - 1, seed) * shape.getMaterial().getReflectivity();
}

Vector3 Renderer::computeRefraction(const Vector3 &P, const Vector3 &v, const Vector3 &intersectionPoint,
                                    const Vector3 &normal, const Shape &shape, const int &order, unsigned int* seed) const
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
        return getPixelColor(intersectionPoint + offset, refractDir, order - 1, seed) * shape.getMaterial().getTransparency();
    }

    const Vector3 reflectDir = (i - normal * 2 * normal.dot(i)).normalized();
    const Vector3 offset = reflectDir * Scene::EPSILON;
    return getPixelColor(intersectionPoint + offset, reflectDir, order - 1, seed) * shape.getMaterial().getTransparency();
}
