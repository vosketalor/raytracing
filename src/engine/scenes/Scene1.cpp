#include "Scene1.h"

#include "LightSource.h"
#include "shapes/OBJ.h"
#include "shapes/Plane.h"
#include "shapes/Sphere.h"
#include "shapes/Triangle.h"


void Scene1::createShapes()
{
    // Plan du sol
    auto plane = std::make_shared<Plane>(Vector3{0, -1, 0}, Vector3{0, 1, 0});
    plane->setColor(Vector3{0.9f, 0.9f, 0.9f});
    plane->setMaterial(Material(0.2, 0.0, 1000, Scene::ETA_AIR, 0.4));
    addShape(plane);

    // Sphère bleue réfléchissante
    auto sphere = std::make_shared<Sphere>(Vector3{0, -0.5f, -4}, 0.5f);
    sphere->setColor(Scene::BLUE);
    sphere->setMaterial(Material::Mirror);
    // sphere->setWireframeEnabled(true);
    addShape(sphere);
    //
    // // Sphère rouge transparente
    // sphere = std::make_shared<Sphere>(Vector3{-0.5f, -0.5f, -3}, 0.5f);
    // sphere->setColor(Scene::RED);
    // sphere->setMaterial(Material(0.0, 0.9, 1000, Scene::ETA_AIR));
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
    // sphere->setColor(Scene::PINK);
    // sphere->setMaterial(Material(0.0, 0.6, 1000, Scene::ETA_WATER));
    // addShape(sphere);

    // Sphère grise
    // sphere = std::make_shared<Sphere>(Vector3{0, -0.5f, 2}, 0.5f);
    // sphere->setColor(Scene::DARK_GRAY);
    // addShape(sphere);

    // auto triangle = std::make_shared<Triangle>(Vector3{0, 0, -5}, Vector3{-1, 1, -6}, Vector3{1, 0, -5});
    // triangle->setColor(Scene::ORANGE);
    // // triangle->setWireframeEnabled(true);
    // addShape(triangle);

    // auto teapot = std::make_shared<OBJ>("res/obj/teapot.obj", Vector3(0, 0, -5));
    // teapot->setColor(Scene::ORANGE);
    // teapot->setMaterial(Material(0.0, 0.3, 1000, Scene::ETA_AIR));
    // teapot->update();
    // addShape(teapot);
}

void Scene1::createLights()
{
    Vector3 position{0, 6, 2};

    Vector3 uDir{1, 0, 0};
    Vector3 vDir{0, 0, 1};

    double width  = 5.0;
    double height = 5.0;

    Vector3 colorDiffuse  {1.0, 1.0, 1.0};
    Vector3 colorSpecular {1.0, 1.0, 1.0};

    auto source = std::make_shared<LightSource>(
        position,
        uDir,
        vDir,
        width,
        height,
        colorDiffuse,
        colorSpecular);

    // auto source = std::make_shared<LightSource>(position, colorDiffuse, colorSpecular);

    source->setIntensity(2.0);

    addLightSource(source);
}