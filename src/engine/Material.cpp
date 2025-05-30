#include "Material.h"

#include "scenes/Scene.h"

const Material Material::Mirror   = Material(1.0, 0.0, 1000.0, Scene::ETA_AIR);
const Material Material::Glass    = Material(0.1, 0.9, 300.0, Scene::ETA_GLASS);
const Material Material::Water    = Material(0.05, 0.95, 100.0, Scene::ETA_WATER);
const Material Material::Metal    = Material(0.85, 0.0, 800.0, Scene::ETA_AIR);
const Material Material::Wood     = Material(0.1, 0.0, 30.0, Scene::ETA_AIR);
const Material Material::Plastic  = Material(0.3, 0.0, 100.0, Scene::ETA_AIR);
const Material Material::Diamond  = Material(0.17, 0.9, 1000.0, Scene::ETA_DIAMOND);
