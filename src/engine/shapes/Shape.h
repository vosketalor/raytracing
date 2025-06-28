#pragma once

#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <typeinfo>

#include "Vector.h"
#include "Camera.h"
#include "GPUShapeData.h"
#include "Material.h"
#include "../textures/Texture.h"
#include "textures/SubTexture.h"

class BoundingBox;
class Scene; // ✅ Forward declaration

/**
 * @brief Abstract base class representing a 3D geometric shape for ray tracing
 */
class Shape {
protected:
    glm::vec3 color_ = {1.0f, 1.0f, 1.0f};
    Material material_;
    bool visible = true;
    std::shared_ptr<BoundingBox> boundingBox;
    std::string texture;
    bool hasTexture_ = false;
    bool wireframeEnabled = false;
    SubTexture coordinatesTextures;

public:
    static std::vector<Material> materials;
    virtual ~Shape() = default;

    glm::vec3 getColor() const { return color_; }
    void setColor(const glm::vec3& col) { color_ = col; }

    Material getMaterial() const { return material_; }
    void setMaterial(const Material& mat) { material_ = mat; }

    virtual void setBoundingBox() = 0;
    std::shared_ptr<BoundingBox> getBoundingBox() const { return boundingBox; }

    std::string getTexture() const { return texture; }
    void setTexture(const std::string& tex) {
        if (!tex.empty()) {
            texture = tex;
            hasTexture_ = true;
        }
    }

    bool hasTexture() const { return hasTexture_; }

    virtual std::string toString() const {
        return typeid(*this).name();
    }

    bool isWireframeEnabled() const { return wireframeEnabled; }
    void setWireframeEnabled(bool isEnabled) { wireframeEnabled = isEnabled; }

    virtual GPU::GPUShapeData toGPU(Scene* scene) const;

    static uint64_t addMaterial(const Material& mat) {
        auto it = std::find(materials.begin(), materials.end(), mat);
        if (it != materials.end()) {
            return std::distance(materials.begin(), it);
        }
        materials.push_back(mat);
        return static_cast<uint64_t>(materials.size() - 1);
    }
};
