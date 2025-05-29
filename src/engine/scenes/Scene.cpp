#include "Scene.h"
const Vector3 Scene::BLACK = Vector3(0, 0, 0);
const Vector3 Scene::WHITE = Vector3(1, 1, 1);
const Vector3 Scene::GRAY = Vector3(0.5, 0.5, 0.5);
const Vector3 Scene::DARK_GRAY = Vector3(0.25, 0.25, 0.25);
const Vector3 Scene::LIGHT_GRAY = Vector3(0.75, 0.75, 0.75);

const Vector3 Scene::RED = Vector3(1, 0, 0);
const Vector3 Scene::GREEN = Vector3(0, 1, 0);
const Vector3 Scene::BLUE = Vector3(0, 0, 1);

const Vector3 Scene::YELLOW = Vector3(1, 1, 0);
const Vector3 Scene::CYAN = Vector3(0, 1, 1);
const Vector3 Scene::MAGENTA = Vector3(1, 0, 1);

const Vector3 Scene::ORANGE = Vector3(1, 0.5, 0);
const Vector3 Scene::PURPLE = Vector3(0.5, 0, 0.5);
const Vector3 Scene::PINK = Vector3(1, 0.5, 0.75);
const Vector3 Scene::BROWN = Vector3(0.6, 0.3, 0.1);


#include "Scene.h"

// Ajoute une forme (version shared_ptr)
void Scene::addShape(const std::shared_ptr<Shape>& shape) {
    shapes.push_back(shape);
}

// Ajoute une forme (version pointeur brut - conversion automatique en shared_ptr)
void Scene::addShape(Shape* shape) {
    shapes.emplace_back(shape);
}

// Ajoute une lumière (version shared_ptr)
void Scene::addLightSource(const std::shared_ptr<LightSource>& lightSource) {
    lightSources.push_back(lightSource);
}

// Ajoute une lumière (version pointeur brut)
void Scene::addLightSource(LightSource* lightSource) {
    lightSources.emplace_back(lightSource);
}

// Modifie la couleur du ciel
void Scene::setSkyColor(const Vector3& color) {
    skyColor = color;
}

// Modifie l'éclairage ambiant
void Scene::setAmbient(const Vector3& ambient) {
    this->ambient = ambient;
}