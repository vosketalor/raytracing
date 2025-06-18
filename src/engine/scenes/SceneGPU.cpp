#include "SceneGPU.h"

#include "shapes/Plane.h"
#include "shapes/Sphere.h"


void SceneGPU::createShapes()
{
    auto plane = std::make_shared<Plane>(Vector3(0, 1, 0), 1);
    plane->setMaterial(Material::PlasticRough);
    this->addShape(plane);

    auto sphere = std::make_shared<Sphere>(glm::vec3{0, 0, -3}, 1);
    sphere->setColor(Scene::YELLOW);
    sphere->setMaterial(Material(0.3,0.7,1000,ETA_WATER, glm::vec3(2.95f, 2.38f, 1.71f), 0.1));
    this->addShape(sphere);

    sphere = std::make_shared<Sphere>(glm::vec3{-2, 0, -4}, 1);
    sphere->setColor(Scene::RED);
    sphere->setMaterial(Material(0,1,1000,Scene::ETA_GLASS, glm::vec3(0.0), 0.01));
    this->addShape(sphere);

    sphere = std::make_shared<Sphere>(glm::vec3{2, 0, -4}, 1);
    sphere->setColor(Scene::BLUE);
    sphere->setMaterial(Material(0.3,0,1000,1.8, glm::vec3(0.0),0.1, 0.5));
    this->addShape(sphere);

    // int nb = 10;
    // for (int i = -nb; i < nb; ++i)
    // {
    //     for (int k = -nb; k < nb; ++k)
    //     {
    //         if (i != 0 || k!=0)
    //         {
    //             auto sphere = std::make_shared<Sphere>(glm::vec3{2*i, 0, 2*k}, 1);
    //             sphere->setColor(Scene::YELLOW);
    //             sphere->setMaterial(Material(0, k%2 ? 1 : 0, 0, 1000, Scene::ETA_AIR));
    //             this->addShape(sphere);
    //         }
    //     }
    // }
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
