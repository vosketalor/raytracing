#include "Scene.h"

void Scene::addShape(const std::shared_ptr<Shape>& shape) {
    shapes.push_back(shape);
}

void Scene::addLightSource(const std::shared_ptr<LightSource>& lightSource) {
    lightSources.push_back(lightSource);
}

void Scene::setSkyColor(const Vector3& color) {
    skyColor = color;
}

void Scene::setAmbient(const Vector3& ambient) {
    this->ambient = ambient;
}