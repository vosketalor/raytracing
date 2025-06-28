#pragma once
#include "Vector.h"
#include <random>
#include <cmath>

#include "GPUMaterial.h"

class Scene;

#ifdef _WIN32
  #include <corecrt_math_defines.h>  // Windows (Visual Studio)
#endif

// ====== MODIFICATION DE Material.h ======
class Material {
protected:
    float reflectivity = 0.0;            ///< [0-1] reflection coefficient
    float transparency = 0.0;            ///< [0-1] transparency coefficient
    float shininess = 100.0;             ///< Phong shininess factor
    float etaReal = 1.0;                     ///< Refractive index
    glm::vec3 etaIm = glm::vec3{0.0, 0.0, 0.0}; ///< Refractive imaginary index
    float roughness = 0.0;               ///< [0-1] surface roughness (0 = perfectly smooth)

    // NOUVEAUX PARAMÈTRES POUR MICROFACETTES
    float metallic = 0.0;                ///< [0-1] métallique (0=diélectrique, 1=conducteur)
    glm::vec3 f0 = glm::vec3{0.04, 0.04, 0.04}; ///< Réflectance à incidence normale (pour diélectriques)

    std::string albedoMap;
    bool hasAlbedoMap_ = false;
    std::string ambientOcclusionMap;
    bool hasAmbientOcclusionMap_ = false;
    std::string normalMap;
    bool hasNormalMap_ = false;
    std::string roughnessMap;
    bool hasRoughnessMap_ = false;
    std::string metalnessMap;
    bool hasMetalnessMap_ = false;
    std::string heightMap;
    bool hasHeightMap_ = false;
public:
    Material() = default;

    Material(const float refl, const float trans, const float shine,
             const float refractiveIndex, const glm::vec3& refractiveImIndex = glm::vec3(0.0,0.0,0.0), const float rough = 0.0,
             const float metal = 0.0, const glm::vec3& fresnel0 = glm::vec3(0.04, 0.04, 0.04))
        : reflectivity(refl), transparency(trans), shininess(shine),
          etaReal(refractiveIndex), etaIm(refractiveImIndex), roughness(rough), metallic(metal), f0(fresnel0) {}

    // Getters et setters existants...
    float getRoughness() const { return roughness; }
    void setRoughness(const float rough) { roughness = rough; }

    // NOUVEAUX GETTERS/SETTERS
    float getMetallic() const { return metallic; }
    void setMetallic(const float metal) { metallic = metal; }

    glm::vec3 getF0() const { return f0; }
    void setF0(const glm::vec3& fresnel0) { f0 = fresnel0; }

    float getEta() const { return etaReal; }
    void setEta(const float refractiveIndex) { etaReal = refractiveIndex; }

    float getReflectivity() const { return reflectivity; }
    void setReflectivity(const float refl) { reflectivity = refl; }

    float getShininess() const { return shininess; }
    void setShininess(const float shine) { shininess = shine; }

    float getTransparency() const { return transparency; }
    void setTransparency(const float trans) { transparency = trans; }

    std::string getAlbedoMap() const { return albedoMap; }
    void setAlbedoMap(const std::string& tex) {
        if (!tex.empty()) {
            albedoMap = tex;
            hasAlbedoMap_ = true;
        }
    }

    bool hasAlbedoMap() const { return hasAlbedoMap_; }

    std::string getAmbientOcclusionMap() const { return ambientOcclusionMap; }
    void setAmbientOcclusionMap(const std::string& tex) {
        if (!tex.empty()) {
            albedoMap = tex;
            hasAlbedoMap_ = true;
        }
    }

    bool hasAmbientOcclusionMap() const { return hasAmbientOcclusionMap_; }

    std::string getNormalMap() const { return normalMap; }
    void setNormalMap(const std::string& tex) {
        if (!tex.empty()) {
            normalMap = tex;
            hasNormalMap_ = true;
        } else {
            normalMap = "";
            hasNormalMap_ = false;
        }
    }
    bool hasNormalMap() const { return hasNormalMap_; }

    std::string getRoughnessMap() const { return roughnessMap; }
    void setRoughnessMap(const std::string& tex) {
        if (!tex.empty()) {
            roughnessMap = tex;
            hasRoughnessMap_ = true;
        } else {
            roughnessMap = "";
            hasRoughnessMap_ = false;
        }
    }
    bool hasRoughnessMap() const { return hasRoughnessMap_; }

    std::string getMetalnessMap() const { return metalnessMap; }
    void setMetalnessMap(const std::string& tex) {
        if (!tex.empty()) {
            metalnessMap = tex;
            hasMetalnessMap_ = true;
        } else {
            metalnessMap = "";
            hasMetalnessMap_ = false;
        }
    }
    bool hasMetalnessMap() const { return hasMetalnessMap_; }

    std::string getHeightMap() const { return heightMap; }
    void setHeightMap(const std::string& tex) {
        if (!tex.empty()) {
            heightMap = tex;
            hasHeightMap_ = true;
        } else {
            heightMap = "";
            hasHeightMap_ = false;
        }
    }
    bool hasHeightMap() const { return hasHeightMap_; }

    GPU::GPUMaterial toGPU(const Scene* scene) const;

    static const Material IronRusty;
    static const Material GoldBrushed;
    static const Material Concrete;
    static const Material CarbonFiber;
    static const Material Marble;
    static const Material Obsidian;
    static const Material Fabric;
    static const Material Leather;
    static const Material Wood;
    static const Material Porcelain;
    static const Material Ceramic;
    static const Material Rubber;
    static const Material PlasticSmooth;
    static const Material Ice;
    static const Material Water;
    static const Material Diamond;
    static const Material Chrome;
    static const Material Aluminum;
    static const Material Iron;
    static const Material Platinum;
    static const Material Copper;
    static const Material Silver;
    static const Material Gold;
    static const Material Mirror;
    static const Material Glass;
    static const Material PlasticRough;

    bool operator==(const Material& other) const
    {
        return reflectivity == other.reflectivity &&
            transparency == other.transparency &&
            shininess == other.shininess &&
            etaReal == other.etaReal &&
            etaIm == other.etaIm &&
            roughness == other.roughness &&
            metallic == other.metallic &&
            f0 == other.f0;
    }
};