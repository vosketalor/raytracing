#include "Shape.h"
#include "scenes/Scene.h"
std::vector<Material> Shape::materials;

GPU::GPUShapeData Shape::toGPU(Scene* scene) const {
    GPU::GPUShapeData data;
    data.color = glm::vec3(color_.x, color_.y, color_.z);
    data.materialIndex = Shape::addMaterial(material_);
    data.wireframeEnabled = wireframeEnabled;
    if (hasTexture())
    {
        data.textureIndex = scene->texture_atlas.getIndex(texture);
    }
    return data;
}