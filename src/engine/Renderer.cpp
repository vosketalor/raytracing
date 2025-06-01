#include "Renderer.h"
#include "LightSource.h"
#include "shapes/Shape.h"

#include <cstdlib>
#include <execution>

#ifdef _WIN32
  #include <corecrt_math_defines.h>  // Windows (Visual Studio)
#endif


static thread_local std::random_device rd;
static thread_local std::mt19937 gen(rd());
static thread_local std::uniform_real_distribution<double> dist(-1.0, 1.0);

#include "Camera.h"

void Renderer::render(std::vector<Vector3> &frameBuffer) const
{
    const Vector3 observer = this->camera_.getPosition();
    const double aspectRatio = static_cast<double>(width) / height;

    // Calcul de la taille de l'écran virtuel en fonction du FOV (en radians)
    const double fovRad = this->camera_.getFov() * M_PI / 180.0;
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
                    Vector3 forward = this->camera_.getDirection();       // direction caméra
                    Vector3 right = this->camera_.getRight();              // vecteur droit
                    Vector3 up = this->camera_.getUp();                     // vecteur haut

                    // Calcul du vecteur direction du rayon dans l'espace monde
                    Vector3 rayDir = (forward + px * right + py * up).normalized();

                    frameBuffer[y * width + x] = getPixelColor(observer, rayDir, 10);
                }
            }
        });
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


Vector3 Renderer::getPixelColor(const Vector3 &P, const Vector3 &v, const int &order) const
{
    const Intersection result = findNearestIntersection(P, v);
    if (!result || result.shape == nullptr)
        return this->scene->getSkyColor();

    const Vector3 intersectionPoint = P + v * result.lambda;
    const Vector3 normal = result.normal;

    // Wireframe check
    if (result.shape->isWireframeEnabled()) {
        const double distance = result.shape->getDistanceNearestEdge(intersectionPoint, this->camera_);
        if (distance < Scene::WIREFRAME_THICKNESS)
            return Scene::WIREFRAME_COLOR;
    }

    Vector3 color = result.shape->getColor();
    color *= this->scene->getAmbient();

    // CHOISIR entre ancien et nouveau modèle d'éclairage
    const Material& material = result.shape->getMaterial();

    // Si le matériau a metallic > 0, utiliser microfacettes, sinon ancien modèle
    if (material.getMetallic() > 0.0 || material.getRoughness() > 0.5) {
        // Nouveau modèle microfacettes
        Vector3 microfacetsLight = computeMicrofacetsLighting(P, v, intersectionPoint, normal, *result.shape);

        // Vérifier que le résultat est valide
        if (std::isfinite(microfacetsLight.x()) && std::isfinite(microfacetsLight.y()) && std::isfinite(microfacetsLight.z())) {
            color += microfacetsLight * result.shape->getColor(); // Multiplier par la couleur
        } else {
            // Fallback vers l'ancien modèle si problème
            color += computeLighting(P, v, intersectionPoint, normal, *result.shape);
        }
    } else {
        // Ancien modèle pour les matériaux simples
        color += computeLighting(P, v, intersectionPoint, normal, *result.shape);
    }

    // Réflexions
    if (order > 0) {
        if (material.getReflectivity() > 0.0 || material.getMetallic() > 0.0) {
            Vector3 reflectionColor;

            if (material.getMetallic() > 0.0) {
                reflectionColor = sampleMicrofacetsReflection(P, v, intersectionPoint, normal, *result.shape, order);
            } else {
                reflectionColor = computeReflection(P, v, intersectionPoint, normal, *result.shape, order);
            }

            color += reflectionColor * std::max(material.getReflectivity(), material.getMetallic());
        }

        // Réfraction (garder l'ancien pour l'instant)
        if (material.getTransparency() > 0.0) {
            color += computeRefraction(P, v, intersectionPoint, normal, *result.shape, order, material.getTransparency());
        }
    }

    return color;
}

Intersection Renderer::findNearestIntersection(const Vector3 &P, const Vector3 &v) const
{
    return this->bvh_.getIntersection(P, v);
}

double Renderer::computeCurvatureBias(const Shape& shape,
                                     const Vector3& intersectionPoint,
                                     const Vector3& normal,
                                     const Vector3& rayDir) const {
    // 1. Composante distance-dépendante (évite les artefacts à grande distance)
    const double distanceBias = 1e-4 * std::max(1.0, intersectionPoint.norm());

    // 2. Composante courbure-dépendante (plus grand biais pour les angles rasant)
    const double cosTheta = std::abs(normal.dot(rayDir));
    const double curvatureBias = 1e-3 * (1.0 - cosTheta); // Ajustez 1e-3 selon vos besoins

    const double edgeDistance = shape.getDistanceNearestEdge(intersectionPoint, camera_);
    const double edgeBias = 1e-4 * std::max(0.0, 1.0 - edgeDistance / Scene::WIREFRAME_THICKNESS);
    return distanceBias + curvatureBias + edgeBias;
}

Vector3 Renderer::computeLighting(const Vector3 &P,
                                  const Vector3 &v,
                                  const Vector3 &intersectionPoint,
                                  const Vector3 &normal,
                                  const Shape &shape) const
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
            Vector3 shadowOrig = intersectionPoint + Ldir * computeCurvatureBias(shape, intersectionPoint, Ldir, v);

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

std::pair<double, double> Renderer::computeFresnelCoefficients(const Vector3& incident,
                                                              const Vector3& normal,
                                                              double etaI,
                                                              double etaT) const
{
    Vector3 i = incident.normalized();
    Vector3 n = normal.normalized();

    double cosI = -n.dot(i);
    bool entering = cosI > 0;

    if (!entering) {
        // Rayon sortant de l'objet
        std::swap(etaI, etaT);
        cosI = -cosI;
        n = -n;
    }

    double eta = etaI / etaT;
    double sinT2 = eta * eta * (1.0 - cosI * cosI);

    // Réflexion totale interne
    if (sinT2 > 1.0) {
        return {1.0, 0.0}; // {réflexion, transmission}
    }

    double cosT = sqrt(1.0 - sinT2);

    // Équations de Fresnel
    double Rs = ((etaI * cosI - etaT * cosT) / (etaI * cosI + etaT * cosT));
    Rs = Rs * Rs;

    double Rp = ((etaT * cosI - etaI * cosT) / (etaT * cosI + etaI * cosT));
    Rp = Rp * Rp;

    double R = (Rs + Rp) * 0.5; // Coefficient de réflexion moyen
    double T = 1.0 - R;         // Coefficient de transmission

    return {R, T};
}

// Modification de computeReflection pour prendre un coefficient
Vector3 Renderer::computeReflection(const Vector3 &P, const Vector3 &v, const Vector3 &intersectionPoint,
                                    const Vector3 &normal, const Shape &shape, const int &order, double fresnelR) const
{
    if (fresnelR <= 0.0)
        return Vector3(0, 0, 0);

    Vector3 reflectDir = (v - normal * 2 * normal.dot(v)).normalized();

    const double roughness = shape.getMaterial().getRoughness();
    if (roughness > 0.0) {
        reflectDir = perturbVector(reflectDir, normal, roughness);

        if (reflectDir.dot(normal) < 0) {
            reflectDir = reflectDir - normal * (2 * reflectDir.dot(normal));
        }
    }

    Vector3 offset = reflectDir * computeCurvatureBias(shape, intersectionPoint, reflectDir, v);
    return getPixelColor(intersectionPoint + offset, reflectDir, order - 1) * fresnelR;
}

// Modification de computeRefraction pour prendre un coefficient
Vector3 Renderer::computeRefraction(const Vector3 &P, const Vector3 &v, const Vector3 &intersectionPoint,
                                    const Vector3 &normal, const Shape &shape, const int &order, double fresnelT) const
{
    if (fresnelT <= 0.0)
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
        const Vector3 refractDir = (i * eta + n * (eta * c1 - c2)).normalized();
        const Vector3 offset = refractDir * computeCurvatureBias(shape, intersectionPoint, refractDir, v);
        return getPixelColor(intersectionPoint + offset, refractDir, order - 1) * fresnelT;
    }

    // Réflexion totale interne - on utilise la réflexion
    const Vector3 reflectDir = (i - n * 2 * n.dot(i)).normalized();
    const Vector3 offset = reflectDir * computeCurvatureBias(shape, intersectionPoint, reflectDir, v);
    return getPixelColor(intersectionPoint + offset, reflectDir, order - 1);
}

Vector3 Renderer::computeMicrofacetsBRDF(const Vector3& viewDir, const Vector3& lightDir,
                                        const Vector3& normal, const Material& material) const
{
    Vector3 halfVector = (viewDir + lightDir).normalized();

    double NdotL = std::max(normal.dot(lightDir), 0.0);
    double NdotV = std::max(normal.dot(viewDir), 0.0);
    double NdotH = std::max(normal.dot(halfVector), 0.0);
    double VdotH = std::max(viewDir.dot(halfVector), 0.0);

    // Si pas d'éclairage, retourner noir
    if (NdotL <= 0.0 || NdotV <= 0.0) {
        return Vector3(0, 0, 0);
    }

    double roughness = std::max(0.01, material.getRoughness()); // Éviter rugosité = 0
    double alpha = roughness * roughness;

    // Distribution normale (D) - GGX
    double a2 = alpha * alpha;
    double denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    double D = a2 / (M_PI * denom * denom);

    // Fonction de géométrie simplifiée (G)
    double k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    double G1L = NdotL / (NdotL * (1.0 - k) + k);
    double G1V = NdotV / (NdotV * (1.0 - k) + k);
    double G = G1L * G1V;

    // Fresnel (F) - Version corrigée
    Vector3 F0 = material.getF0();
    if (material.getMetallic() > 0.0) {
        // Pour les métaux, utiliser la couleur de l'objet comme F0
        F0 = F0 * (1.0 - material.getMetallic()) +
             Vector3(1.0, 1.0, 1.0) * material.getMetallic(); // Blanc pour simplicité
    }

    Vector3 F = F0 + (Vector3(1.0, 1.0, 1.0) - F0) * std::pow(1.0 - VdotH, 5.0);

    // BRDF Cook-Torrance
    Vector3 numerator = D * G * F;
    double denominator = 4.0 * NdotV * NdotL + 0.001;
    Vector3 specular = numerator / denominator;

    // Composante diffuse Lambert
    Vector3 kS = F;
    Vector3 kD = Vector3(1.0, 1.0, 1.0) - kS;
    kD = kD * (1.0 - material.getMetallic()); // Pas de diffuse pour les métaux

    Vector3 diffuse = kD / M_PI;

    return diffuse + specular;
}


Vector3 Renderer::computeMicrofacetsLighting(const Vector3& P, const Vector3& v,
                                           const Vector3& intersectionPoint,
                                           const Vector3& normal, const Shape& shape) const
{
    Vector3 result(0, 0, 0);
    Vector3 viewDir = (P - intersectionPoint).normalized();

    // RÉDUIRE le nombre d'échantillons pour débugger
    const int numSamples = 4;

    for (const auto& lightPtr : scene->getLightSources()) {
        const LightSource& L = *lightPtr;
        Vector3 accumLight(0, 0, 0);

        for (int i = 0; i < numSamples; ++i) {
            Vector3 samplePos = L.samplePointOnArea();
            Vector3 Lvec = samplePos - intersectionPoint;
            double Ldist = Lvec.norm();
            Vector3 Ldir = Lvec / Ldist;

            // Test d'ombre simple
            Vector3 shadowOrig = intersectionPoint + normal * 0.001; // Offset simple
            bool inShadow = isInShadow(shadowOrig, Ldir, Ldist);

            if (inShadow) continue;

            // BRDF microfacettes
            Vector3 brdf = computeMicrofacetsBRDF(viewDir, Ldir, normal, shape.getMaterial());

            // Vérifier que la BRDF n'est pas NaN ou infinie
            if (!std::isfinite(brdf.x()) || !std::isfinite(brdf.y()) || !std::isfinite(brdf.z())) {
                continue;
            }

            double NdotL = std::max(0.0, normal.dot(Ldir));
            Vector3 radiance = L.getColorDiffuse() * L.getIntensity();

            // Atténuation par distance
            double attenDist = computeAttenuation(Ldist);

            accumLight += brdf * radiance * NdotL * attenDist;
        }

        result += accumLight / (double)numSamples;
    }

    return result;
}

Vector3 Renderer::sampleMicrofacetsReflection(const Vector3& P, const Vector3& v,
                                             const Vector3& intersectionPoint,
                                             const Vector3& normal, const Shape& shape,
                                             const int& order) const
{
    if (order <= 0) return Vector3(0, 0, 0);

    const Material& material = shape.getMaterial();

    // RÉFLEXION SIMPLE pour commencer (comme avant)
    Vector3 reflectDir = (v - normal * 2.0 * normal.dot(v)).normalized();

    // Ajouter un peu de perturbation basée sur la rugosité
    double roughness = material.getRoughness();
    if (roughness > 0.1) {
        reflectDir = perturbVector(reflectDir, normal, roughness * 0.5);
    }

    // Vérifier que la direction est correcte
    if (reflectDir.dot(normal) <= 0.0) {
        return Vector3(0, 0, 0);
    }

    Vector3 offset = reflectDir * computeCurvatureBias(shape, intersectionPoint, reflectDir, v);
    Vector3 reflectedColor = getPixelColor(intersectionPoint + offset, reflectDir, order - 1);

    // Coefficient de réflexion basé sur Fresnel simple
    Vector3 viewDir = -v;
    double cosTheta = std::max(0.0, viewDir.dot(normal));
    Vector3 F0 = material.getF0();
    Vector3 F = F0 + (Vector3(1.0, 1.0, 1.0) - F0) * std::pow(1.0 - cosTheta, 5.0);

    return reflectedColor * F;
}
