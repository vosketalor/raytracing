// Modification de Scene1.cpp pour tester Fresnel

#include "SceneFresnel.h"

#include "shapes/Plane.h"
#include "shapes/Sphere.h"

void SceneFresnel::createShapes()
{
    // Plan du sol
    auto plane = std::make_shared<Plane>(Vector3{0, -1, 0}, Vector3{0, 1, 0});
    plane->setColor(Vector3{0.9f, 0.9f, 0.9f});
    plane->setMaterial(Material(0.2, 0.0, 1000, Scene::ETA_AIR, 0.4));
    addShape(plane);

    // Sphère métallique réfléchissante (pas de Fresnel, réflexion pure)
    auto sphereMirror = std::make_shared<Sphere>(Vector3{-2, -0.5f, -4}, 0.5f);
    sphereMirror->setColor(Vector3{0.8f, 0.8f, 0.9f});
    sphereMirror->setMaterial(Material::Mirror); // reflectivity=0.9, transparency=0
    addShape(sphereMirror);

    // Sphère de verre (Fresnel actif)
    auto sphereGlass = std::make_shared<Sphere>(Vector3{0, -0.5f, -4}, 0.5f);
    sphereGlass->setColor(Vector3{0.95f, 0.95f, 0.95f});
    // Matériau diélectrique : reflectivity=0, transparency=0.9, eta=1.5 (verre)
    sphereGlass->setMaterial(Material(0.0, 0.9, 1000, 1.5, 0.0));
    addShape(sphereGlass);

    // Sphère d'eau (Fresnel avec indice différent)
    auto sphereWater = std::make_shared<Sphere>(Vector3{2, -0.5f, -4}, 0.5f);
    sphereWater->setColor(Vector3{0.9f, 0.95f, 1.0f});
    // Matériau diélectrique : reflectivity=0, transparency=0.8, eta=1.33 (eau)
    sphereWater->setMaterial(Material(0.0, 0.8, 1000, 1.33, 0.0));
    addShape(sphereWater);

    // Sphère de diamant (haut indice de réfraction)
    auto sphereDiamond = std::make_shared<Sphere>(Vector3{1, 0.5f, -6}, 0.5f);
    sphereDiamond->setColor(Vector3{0.98f, 0.98f, 0.98f});
    // Matériau diélectrique : reflectivity=0, transparency=0.95, eta=2.42 (diamant)
    sphereDiamond->setMaterial(Material(0.0, 0.95, 1000, 2.42, 0.0));
    addShape(sphereDiamond);

    // Sphère verte de référence (matériau diffus)
    auto sphereGreen = std::make_shared<Sphere>(Vector3{-1, 0, -6}, 1.0f);
    sphereGreen->setColor(Scene::GREEN);
    sphereGreen->setMaterial(Material(0.0, 0.0, 100, Scene::ETA_AIR, 0.0));
    addShape(sphereGreen);
}

void SceneFresnel::createLights()
{
    Vector3 position{0, 6, 2};

    Vector3 uDir{1, 0, 0};
    Vector3 vDir{0, 0, 1};

    double width  = 5.0;
    double height = 5.0;

    Vector3 colorDiffuse  {1.0, 1.0, 1.0};
    Vector3 colorSpecular {1.0, 1.0, 1.0};

    const auto source = std::make_shared<LightSource>(
        position,
        uDir,
        vDir,
        width,
        height,
        colorDiffuse,
        colorSpecular);

    source->setIntensity(2.0);

    addLightSource(source);
}
