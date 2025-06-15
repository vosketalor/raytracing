#pragma once
#include "Vector.h"
#include <random>
#include <cmath>

#include "GPUMaterial.h"

#ifdef _WIN32
  #include <corecrt_math_defines.h>  // Windows (Visual Studio)
#endif

// ====== MODIFICATION DE Material.h ======
class Material {
protected:
    float reflectivity = 0.0;            ///< [0-1] reflection coefficient
    float transparency = 0.0;            ///< [0-1] transparency coefficient
    float shininess = 100.0;             ///< Phong shininess factor
    float eta = 1.0;                     ///< Refractive index
    float roughness = 0.0;               ///< [0-1] surface roughness (0 = perfectly smooth)

    // NOUVEAUX PARAMÈTRES POUR MICROFACETTES
    float metallic = 0.0;                ///< [0-1] métallique (0=diélectrique, 1=conducteur)
    Vector3 f0 = Vector3(0.04, 0.04, 0.04); ///< Réflectance à incidence normale (pour diélectriques)

public:
    Material() = default;

    Material(const float refl, const float trans, const float shine,
             const float refractiveIndex, const float rough = 0.0,
             const float metal = 0.0, const Vector3& fresnel0 = Vector3(0.04, 0.04, 0.04))
        : reflectivity(refl), transparency(trans), shininess(shine),
          eta(refractiveIndex), roughness(rough), metallic(metal), f0(fresnel0) {}

    // Getters et setters existants...
    float getRoughness() const { return roughness; }
    void setRoughness(const float rough) { roughness = rough; }

    // NOUVEAUX GETTERS/SETTERS
    float getMetallic() const { return metallic; }
    void setMetallic(const float metal) { metallic = metal; }

    Vector3 getF0() const { return f0; }
    void setF0(const Vector3& fresnel0) { f0 = fresnel0; }

    float getEta() const { return eta; }
    void setEta(const float refractiveIndex) { eta = refractiveIndex; }

    float getReflectivity() const { return reflectivity; }
    void setReflectivity(const float refl) { reflectivity = refl; }

    float getShininess() const { return shininess; }
    void setShininess(const float shine) { shininess = shine; }

    float getTransparency() const { return transparency; }
    void setTransparency(const float trans) { transparency = trans; }

    GPU::GPUMaterial toGPU() const;

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
            eta == other.eta &&
            roughness == other.roughness &&
            metallic == other.metallic &&
            f0 == other.f0;
    }
};