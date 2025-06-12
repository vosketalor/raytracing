#include "SceneGPU.h"

#include "shapes/Sphere.h"


void SceneGPU::createShapes()
{
    auto sphere = std::make_shared<Sphere>(Vector3(0, 0, -3), 1);
    this->addShape(sphere);
}

void SceneGPU::createLights()
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
