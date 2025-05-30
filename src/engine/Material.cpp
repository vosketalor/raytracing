#include "Material.h"

#include "scenes/Scene.h"

const Material Material::Mirror   = Material(1.0, 0.0, 1000.0, Scene::ETA_AIR, 0.0);
const Material Material::Glass    = Material(0.1, 0.9, 300.0, Scene::ETA_GLASS, 0.05);
const Material Material::Water    = Material(0.05, 0.95, 100.0, Scene::ETA_WATER, 0.2);
const Material Material::Metal    = Material(0.85, 0.0, 800.0, Scene::ETA_AIR, 0.15);
const Material Material::Wood     = Material(0.1, 0.0, 30.0, Scene::ETA_AIR, 0.8);
const Material Material::Plastic  = Material(0.3, 0.0, 100.0, Scene::ETA_AIR, 0.3);
const Material Material::Diamond  = Material(0.17, 0.9, 1000.0, Scene::ETA_DIAMOND, 0.01);
