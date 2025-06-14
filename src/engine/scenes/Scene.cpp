#include "Scene.h"

void Scene::addShape(const std::shared_ptr<Shape>& shape) {
    shapes.push_back(shape);
}

void Scene::addLightSource(const std::shared_ptr<LightSource>& lightSource) {
    lightSources.push_back(lightSource);
}

void Scene::setSkyColor(const glm::vec3& color) {
    skyColor = color;
}

void Scene::setAmbient(const glm::vec3& ambient) {
    this->ambient = ambient;
}