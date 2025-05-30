#include "Scene1.h"

#include "LightSource.h"
#include "shapes/Plane.h"
#include "shapes/Sphere.h"


void Scene1::createShapes()
{
    // Plan du sol
    const auto plane = std::make_shared<Plane>(Vector3{0, -1, 0}, Vector3{0, 1, 0});
    plane->setColor(Vector3{0.3f, 0.3f, 0.3f});
    plane->setReflectivity(0.2f);
    addShape(plane);
    //
    // Sphère bleue réfléchissante
    auto sphere = std::make_shared<Sphere>(Vector3{0, -0.5f, -4}, 0.5f);
    sphere->setColor(Scene::BLUE);
    sphere->setReflectivity(0.9f);
    addShape(sphere);
    //
    // // Sphère rouge transparente
    // sphere = std::make_shared<Sphere>(Vector3{-0.5f, -0.5f, -3}, 0.5f);
    // sphere->setColor(Scene::RED);
    // sphere->setTransparency(0.9f);
    // sphere->setEta(Scene::ETA_AIR);
    // addShape(sphere);
    //
    // // Sphère verte simple
    // sphere = std::make_shared<Sphere>(Vector3{1, 0, -6}, 1.0f);
    // sphere->setColor(Scene::GREEN);
    // addShape(sphere);
    //
    // // Sphère texturée
    // sphere = std::make_shared<Sphere>(Vector3{1, -0.75f, -3}, 0.25f);
    // sphere->setTexture(std::make_shared<Texture>("textures/beachball.jpg"));
    // addShape(sphere);
    //
    // // Sphère rose transparente (eau)
    // sphere = std::make_shared<Sphere>(Vector3{-1.5f, 0, -5.5f}, 1.0f);
    // sphere->setTransparency(0.6f);
    // sphere->setEta(Scene::ETA_WATER);
    // sphere->setColor(Scene::PINK);
    // addShape(sphere);
    //
    // // Sphère grise
    // sphere = std::make_shared<Sphere>(Vector3{0, -0.5f, 2}, 0.5f);
    // sphere->setColor(Scene::DARK_GRAY);
    // addShape(sphere);
}

void Scene1::createLights()
{
    // Source lumineuse principale
    const auto source = std::make_shared<LightSource>(
        Vector3{0, 5, -3},
        Vector3{0.7f, 0.7f, 0.7f},
        Vector3{1.0f, 1.0f, 1.0f}
    );
    source->setIntensity(1.0f);
    addLightSource(source);
}