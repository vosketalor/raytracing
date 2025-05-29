#pragma once
#include <memory>
#include <string>
#include "Vector.h"
// #include "Texture.h"
#include "../Intersection.h"

class BoundingBox;

/**
 * @brief Abstract base class representing a 3D geometric shape for ray tracing
 */
class Shape {
protected:
    Vector3 color = {1.0f, 1.0f, 1.0f};  ///< Base color (if no texture)
    float reflectivity = 0.0f;            ///< [0-1] reflection coefficient
    float transparency = 0.0f;            ///< [0-1] transparency coefficient
    float shininess = 100.0f;             ///< Phong shininess factor
    float eta = 1.0f;                     ///< Refractive index
    bool visible = true;                 ///< Visibility flag
    // std::shared_ptr<BoundingBox> boundingBox; ///< Acceleration structure
    // std::shared_ptr<Texture> texture = nullptr; ///< Optional texture
    // bool hasTexture = false;               ///< Texture presence flag

public:
    virtual ~Shape() = default;

    // Core functionality
    virtual Intersection getIntersection(const Vector3& P, const Vector3& v) const = 0;
    virtual Vector2 getTextureCoordinates(const Vector3& intersection) const = 0;

    // Transformations
    // virtual void scale(float scale) = 0;
    // virtual void rotate(float angle, const Vector3& axis) = 0;
    // void rotate(const float angle) { rotate(angle, Vector3{0,1,0}); }

    // Bounding volume
    // virtual void setBoundingBox() = 0;
    // std::shared_ptr<BoundingBox> getBoundingBox() const { return boundingBox; }

    // Getters/Setters
    bool isVisible() const { return visible; }
    void setVisible(const bool isVisible) { visible = isVisible; }

    Vector3 getColor() const { return color; }
    void setColor(const Vector3& col) { color = col; }

    float getReflectivity() const { return reflectivity; }
    void setReflectivity(const float refl) { reflectivity = refl; }

    float getTransparency() const { return transparency; }
    void setTransparency(const float trans) { transparency = trans; }

    float getShininess() const { return shininess; }
    void setShininess(const float shine) { shininess = shine; }

    float getEta() const { return eta; }
    void setEta(const float refractiveIndex) { eta = refractiveIndex; }

    // std::shared_ptr<Texture> getTexture() const { return texture; }
    // void setTexture(std::shared_ptr<Texture> tex) {
    //     if (tex) {
    //         texture = tex;
    //         hasTexture = true;
    //     }
    // }
    //
    // bool hasTexture() const { return hasTexture; }

    virtual std::string toString() const {
        return typeid(*this).name();
    }
};