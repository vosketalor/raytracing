#include "SceneGPU.h"

#include "shapes/Plane.h"
#include "shapes/Sphere.h"


void SceneGPU::createShapes()
{
    auto plane = std::make_shared<Plane>(Vector3(0, 1, 0), 1);
    plane->setMaterial(Material::Concrete);
    this->addShape(plane);

    auto sphere = std::make_shared<Sphere>(Vector3(0, 0, -3), 1);
    sphere->setColor(Scene::YELLOW);
    sphere->setMaterial(Material::Wood);
    this->addShape(sphere);

    sphere = std::make_shared<Sphere>(Vector3(-2, 0, -4), 1);
    sphere->setColor(Scene::RED);
    sphere->setMaterial(Material::IronRusty);
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
