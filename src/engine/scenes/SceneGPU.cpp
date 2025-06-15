#include "SceneGPU.h"

#include "shapes/Plane.h"
#include "shapes/Sphere.h"


void SceneGPU::createShapes()
{
    auto plane = std::make_shared<Plane>(Vector3(0, 1, 0), 1);
    plane->setMaterial(Material(0.3f, 0, 100, Scene::ETA_WATER));
    this->addShape(plane);

    auto sphere = std::make_shared<Sphere>(Vector3(0, 0, -3), 1);
    sphere->setColor(Scene::ORANGE);
    sphere->setMaterial(Material(0, 0.5f, 1000, Scene::ETA_WATER));
    this->addShape(sphere);
}

void SceneGPU::createLights()
{
    Vector3 position{0, 6, 0};

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

    source->setIntensity(1.0);

    addLightSource(source);
}
