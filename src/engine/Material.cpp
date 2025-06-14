#include "Material.h"

#include "scenes/Scene.h"

// const Material Material::Mirror   = Material(1.0, 0.0, 1000.0, Scene::ETA_AIR, 0.0);
// const Material Material::Glass    = Material(0.1, 0.9, 300.0, Scene::ETA_GLASS, 0.05);
// const Material Material::Water    = Material(0.05, 0.95, 100.0, Scene::ETA_WATER, 0.2);
// const Material Material::Metal    = Material(0.85, 0.0, 800.0, Scene::ETA_AIR, 0.15);
// const Material Material::Wood     = Material(0.1, 0.0, 30.0, Scene::ETA_AIR, 0.8);
// const Material Material::Plastic  = Material(0.3, 0.0, 100.0, Scene::ETA_AIR, 0.3);
// const Material Material::Diamond  = Material(0.17, 0.9, 1000.0, Scene::ETA_DIAMOND, 0.01);
const Material Material::Mirror = Material(0.9f, 0.f, 1000.f, 1.f, 0.05f, 1.f, Vector3(0.95, 0.95, 0.95));
const Material Material::Glass = Material(0.f, 0.9f, 1000.f, 1.5f, 0.02f, 0.f, Vector3(0.04, 0.04, 0.04));
const Material Material::MetalGold = Material(0.8f, 0.0f, 1000.f, 1.f, 0.1f, 1.f, Vector3(1.0, 0.86, 0.57));
const Material Material::MetalCopper = Material(0.8f, 0.0f, 1000.f, 1.f, 0.15f, 1.f, Vector3(0.95, 0.64, 0.54));
const Material Material::PlasticRough = Material(0.2f, 0.0f, 100.f, 1.f, 0.8f, 0.f, Vector3(0.04, 0.04, 0.04));

GPU::GPUMaterial Material::toGPU() const
{
    GPU::GPUMaterial data;
    data.reflectivity = reflectivity;
    data.transparency = transparency;
    data.shininess = shininess;
    data.eta = eta;
    data.roughness = roughness;
    data.metallic = metallic;
    data.f0 = glm::vec3(f0.x(), f0.y(), f0.z());
    data._pad = 0;
    return data;
}