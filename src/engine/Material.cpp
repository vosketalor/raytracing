#include "Material.h"

#include "scenes/Scene.h"



GPU::GPUMaterial Material::toGPU(const Scene* scene) const
{
    GPU::GPUMaterial data;
    data.reflectivity = reflectivity;
    data.transparency = transparency;
    data.shininess = shininess;
    data.eta = etaReal;
    data.roughness = roughness;
    data.metallic = metallic;
    data.f0 = f0;
    data.etaIm = etaIm;

    if (hasAlbedoMap())
    {
        data.albedoMapId = scene->texture_atlas.getIndex(albedoMap);
    }

    // Pour l'Ambient Occlusion Map
    if (hasAmbientOcclusionMap()) {
        data.ambientOcclusionMapId = scene->texture_atlas.getIndex(ambientOcclusionMap);
    }

    // Pour la Normal Map
    if (hasNormalMap()) {
        data.normalMapId = scene->texture_atlas.getIndex(normalMap);
    }

    // Pour la Roughness Map
    if (hasRoughnessMap()) {
        data.roughnessMapId = scene->texture_atlas.getIndex(roughnessMap);
    }

    // Pour la Metalness Map
    if (hasMetalnessMap()) {
        data.metalnessMapId = scene->texture_atlas.getIndex(metalnessMap);
    }

    // Pour la Height Map
    if (hasHeightMap()) {
        data.heightMapId = scene->texture_atlas.getIndex(heightMap);
    }

    return data;
}

// === MÉTAUX PRÉCIEUX ===
const Material Material::Gold = Material(
    0.9f,                               // reflectivity
    0.0f,                               // transparency
    200.0f,                             // shininess
    0.47f,                              // eta (partie réelle)
    glm::vec3(2.05f, 2.05f, 2.05f),     // etaIm (partie imaginaire pour l'or)
    0.02f,                              // roughness (poli)
    1.0f,                               // metallic
    glm::vec3(1.00f, 0.86f, 0.57f)      // f0 - couleur dorée
);

const Material Material::Silver = Material(
    0.95f,                              // reflectivity
    0.0f,                               // transparency
    250.0f,                             // shininess
    0.155f,                             // eta (partie réelle)
    glm::vec3(3.52f, 3.52f, 3.52f),     // etaIm (partie imaginaire pour l'argent)
    0.01f,                              // roughness (très poli)
    1.0f,                               // metallic
    glm::vec3(0.95f, 0.95f, 0.95f)      // f0 - blanc argenté
);

const Material Material::Copper = Material(
    0.85f,                              // reflectivity
    0.0f,                               // transparency
    180.0f,                             // shininess
    1.1f,                               // eta (partie réelle)
    glm::vec3(2.6f, 2.6f, 2.6f),        // etaIm (partie imaginaire pour le cuivre)
    0.05f,                              // roughness
    1.0f,                               // metallic
    glm::vec3(0.95f, 0.64f, 0.54f)      // f0 - rouge cuivré
);

const Material Material::Platinum = Material(
    0.90f,                              // reflectivity
    0.0f,                               // transparency
    220.0f,                             // shininess
    2.33f,                              // eta (partie réelle)
    glm::vec3(4.0f, 4.0f, 4.0f),        // etaIm (partie imaginaire pour le platine)
    0.02f,                              // roughness
    1.0f,                               // metallic
    glm::vec3(0.73f, 0.71f, 0.68f)      // f0 - gris platine
);

// === MÉTAUX INDUSTRIELS ===
const Material Material::Iron = Material(
    0.70f,                              // reflectivity
    0.0f,                               // transparency
    120.0f,                             // shininess
    2.95f,                              // eta (partie réelle)
    glm::vec3(3.0f, 3.0f, 3.0f),        // etaIm (partie imaginaire pour le fer)
    0.15f,                              // roughness (un peu rugueuse)
    1.0f,                               // metallic
    glm::vec3(0.56f, 0.57f, 0.58f)      // f0 - gris fer
);

const Material Material::Aluminum = Material(
    0.85f,                              // reflectivity
    0.0f,                               // transparency
    150.0f,                             // shininess
    1.44f,                              // eta (partie réelle)
    glm::vec3(5.0f, 5.0f, 5.0f),        // etaIm (partie imaginaire pour l'aluminium)
    0.08f,                              // roughness
    1.0f,                               // metallic
    glm::vec3(0.91f, 0.92f, 0.92f)      // f0 - blanc aluminium
);

const Material Material::Chrome = Material(
    0.95f,                              // reflectivity
    0.0f,                               // transparency
    300.0f,                             // shininess
    3.2f,                               // eta (partie réelle)
    glm::vec3(3.3f, 3.3f, 3.3f),        // etaIm (partie imaginaire pour le chrome)
    0.01f,                              // roughness (très poli)
    1.0f,                               // metallic
    glm::vec3(0.54f, 0.55f, 0.55f)      // f0 - gris chrome
);

// === VERRES ET TRANSPARENTS ===
const Material Material::Glass = Material(
    0.08f,                              // reflectivity (Fresnel)
    0.95f,                              // transparency
    200.0f,                             // shininess
    1.52f,                              // eta (verre standard)
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm (pas d'absorption pour verre transparent)
    0.0f,                               // roughness (lisse)
    0.0f,                               // metallic
    glm::vec3(0.04f, 0.04f, 0.04f)      // f0 diélectrique
);

const Material Material::Diamond = Material(
    0.17f,                              // reflectivity (Fresnel élevé)
    0.98f,                              // transparency
    500.0f,                             // shininess (très brillant)
    2.42f,                              // eta (diamant)
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm (pas d'absorption pour diamant pur)
    0.0f,                               // roughness
    0.0f,                               // metallic
    glm::vec3(0.17f, 0.17f, 0.17f)      // f0 élevé pour diamant
);

const Material Material::Water = Material(
    0.02f,                              // reflectivity (Fresnel faible)
    0.90f,                              // transparency
    100.0f,                             // shininess
    1.33f,                              // eta (eau)
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm (eau pure n'absorbe pas)
    0.02f,                              // roughness (légères ondulations)
    0.0f,                               // metallic
    glm::vec3(0.02f, 0.02f, 0.02f)      // f0 très faible
);

const Material Material::Ice = Material(
    0.03f,                              // reflectivity
    0.85f,                              // transparency
    150.0f,                             // shininess
    1.31f,                              // eta (glace)
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm (glace pure n'absorbe pas)
    0.05f,                              // roughness (surface irrégulière)
    0.0f,                               // metallic
    glm::vec3(0.03f, 0.03f, 0.03f)      // f0
);

// === PLASTIQUES ===
const Material Material::PlasticSmooth = Material(
    0.05f,                              // reflectivity
    0.0f,                               // transparency
    80.0f,                              // shininess
    1.46f,                              // eta (plastique)
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm (plastique opaque)
    0.0f,                               // roughness (lisse)
    0.0f,                               // metallic
    glm::vec3(0.04f, 0.04f, 0.04f)      // f0 diélectrique
);

const Material Material::PlasticRough = Material(
    0.03f,                              // reflectivity
    0.0f,                               // transparency
    40.0f,                              // shininess
    1.46f,                              // eta
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm
    0.4f,                               // roughness (rugueux)
    0.0f,                               // metallic
    glm::vec3(0.04f, 0.04f, 0.04f)      // f0
);

const Material Material::Rubber = Material(
    0.01f,                              // reflectivity (très faible)
    0.0f,                               // transparency
    20.0f,                              // shininess (mat)
    1.52f,                              // eta
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm
    0.8f,                               // roughness (très rugueux)
    0.0f,                               // metallic
    glm::vec3(0.04f, 0.04f, 0.04f)      // f0
);

// === CÉRAMIQUES ET PORCELAINES ===
const Material Material::Ceramic = Material(
    0.12f,                              // reflectivity
    0.0f,                               // transparency
    150.0f,                             // shininess
    1.62f,                              // eta (céramique)
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm
    0.02f,                              // roughness (émaillée)
    0.0f,                               // metallic
    glm::vec3(0.08f, 0.08f, 0.08f)      // f0 plus élevé que plastique
);

const Material Material::Porcelain = Material(
    0.15f,                              // reflectivity
    0.0f,                               // transparency
    200.0f,                             // shininess
    1.54f,                              // eta
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm
    0.01f,                              // roughness (très lisse)
    0.0f,                               // metallic
    glm::vec3(0.08f, 0.08f, 0.08f)      // f0
);

// === MATÉRIAUX ORGANIQUES ===
const Material Material::Wood = Material(
    0.02f,                              // reflectivity (très faible)
    0.0f,                               // transparency
    30.0f,                              // shininess (mat)
    1.54f,                              // eta
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm
    0.6f,                               // roughness (fibre du bois)
    0.0f,                               // metallic
    glm::vec3(0.04f, 0.04f, 0.04f)      // f0
);

const Material Material::Leather = Material(
    0.03f,                              // reflectivity
    0.0f,                               // transparency
    25.0f,                              // shininess
    1.5f,                               // eta
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm
    0.7f,                               // roughness (texture cuir)
    0.0f,                               // metallic
    glm::vec3(0.04f, 0.04f, 0.04f)      // f0
);

const Material Material::Fabric = Material(
    0.01f,                              // reflectivity (très mat)
    0.0f,                               // transparency
    10.0f,                              // shininess (très mat)
    1.46f,                              // eta
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm
    0.9f,                               // roughness (très rugueux)
    0.0f,                               // metallic
    glm::vec3(0.04f, 0.04f, 0.04f)      // f0
);

// === MATÉRIAUX SPÉCIAUX ===
const Material Material::Mirror = Material(
    0.98f,                              // reflectivity (quasi parfait)
    0.0f,                               // transparency
    1000.0f,                            // shininess (parfait)
    1.0f,                               // eta (pas utilisé pour miroir)
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm
    0.0f,                               // roughness
    1.0f,                               // metallic (se comporte comme métal)
    glm::vec3(0.95f, 0.95f, 0.95f)      // f0 très élevé
);

const Material Material::Obsidian = Material(
    0.15f,                              // reflectivity
    0.1f,                               // transparency (légèrement translucide)
    300.0f,                             // shininess (très poli naturellement)
    1.48f,                              // eta (verre volcanique)
    glm::vec3(0.01f, 0.01f, 0.01f),     // etaIm (légère absorption)
    0.01f,                              // roughness (naturellement lisse)
    0.0f,                               // metallic
    glm::vec3(0.08f, 0.08f, 0.08f)      // f0
);

const Material Material::Marble = Material(
    0.08f,                              // reflectivity
    0.05f,                              // transparency (légèrement translucide)
    100.0f,                             // shininess (poli)
    1.54f,                              // eta (calcaire)
    glm::vec3(0.01f, 0.01f, 0.01f),     // etaIm (légère absorption)
    0.05f,                              // roughness (poli mais pas parfait)
    0.0f,                               // metallic
    glm::vec3(0.06f, 0.06f, 0.06f)      // f0
);

// === MATÉRIAUX COMPOSITES ===
const Material Material::CarbonFiber = Material(
    0.25f,                              // reflectivity (semi-métallique)
    0.0f,                               // transparency
    150.0f,                             // shininess
    1.7f,                               // eta
    glm::vec3(0.5f, 0.5f, 0.5f),        // etaIm (absorption modérée)
    0.1f,                               // roughness (texture tissée)
    0.5f,                               // metallic (composite)
    glm::vec3(0.12f, 0.12f, 0.12f)      // f0 intermédiaire
);

const Material Material::Concrete = Material(
    0.02f,                              // reflectivity (très mat)
    0.0f,                               // transparency
    20.0f,                              // shininess
    1.55f,                              // eta
    glm::vec3(0.0f, 0.0f, 0.0f),        // etaIm
    0.8f,                               // roughness (très rugueux)
    0.0f,                               // metallic
    glm::vec3(0.04f, 0.04f, 0.04f)      // f0
);

// === VARIATIONS RUGUEUSES DES MÉTAUX ===
const Material Material::GoldBrushed = Material(
    0.75f,                              // reflectivity (réduite par rugosité)
    0.0f,                               // transparency
    80.0f,                              // shininess (réduite)
    0.47f,                              // eta
    glm::vec3(2.05f, 2.05f, 2.05f),     // etaIm
    0.3f,                               // roughness (brossé)
    1.0f,                               // metallic
    glm::vec3(1.00f, 0.86f, 0.57f)      // f0 identique à l'or poli
);

const Material Material::IronRusty = Material(
    0.25f,                              // reflectivity (très réduite)
    0.0f,                               // transparency
    30.0f,                              // shininess (très mat)
    2.95f,                              // eta
    glm::vec3(3.0f, 3.0f, 3.0f),        // etaIm
    0.9f,                               // roughness (rouille = très rugueux)
    0.7f,                               // metallic (partiellement oxydé)
    glm::vec3(0.45f, 0.35f, 0.25f)      // f0 modifié (couleur rouille)
);