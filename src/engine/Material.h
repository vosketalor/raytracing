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
             const double refractiveIndex, const double rough = 0.0,
             const double metal = 0.0, const Vector3& fresnel0 = Vector3(0.04, 0.04, 0.04))
        : reflectivity(refl), transparency(trans), shininess(shine),
          eta(refractiveIndex), roughness(rough), metallic(metal), f0(fresnel0) {}

    // Getters et setters existants...
    double getRoughness() const { return roughness; }
    void setRoughness(const double rough) { roughness = rough; }

    // NOUVEAUX GETTERS/SETTERS
    double getMetallic() const { return metallic; }
    void setMetallic(const double metal) { metallic = metal; }

    Vector3 getF0() const { return f0; }
    void setF0(const Vector3& fresnel0) { f0 = fresnel0; }

    double getEta() const { return eta; }
    void setEta(const double refractiveIndex) { eta = refractiveIndex; }

    double getReflectivity() const { return reflectivity; }
    void setReflectivity(const double refl) { reflectivity = refl; }

    double getShininess() const { return shininess; }
    void setShininess(const double shine) { shininess = shine; }

    double getTransparency() const { return transparency; }
    void setTransparency(const double trans) { transparency = trans; }

    GPU::GPUMaterial toGPU() const;

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


    // MÉTHODES POUR MICROFACETTES

    /// Distribution normale de Trowbridge-Reitz (GGX)
    // double distributionGGX(const Vector3& normal, const Vector3& halfVector, double alpha) const {
    //     double a2 = alpha * alpha;
    //     double NdotH = std::max(normal.dot(halfVector), 0.0);
    //     double NdotH2 = NdotH * NdotH;
    //
    //     double denom = NdotH2 * (a2 - 1.0) + 1.0;
    //     denom = M_PI * denom * denom;
    //
    //     return a2 / denom;
    // }
    //
    // /// Fonction de géométrie de Smith pour GGX
    // double geometrySchlickGGX(double NdotV, double alpha) const {
    //     double r = alpha + 1.0;
    //     double k = (r * r) / 8.0; // pour direct lighting
    //
    //     double denom = NdotV * (1.0 - k) + k;
    //     return NdotV / denom;
    // }
    //
    // double geometrySmith(const Vector3& normal, const Vector3& viewDir,
    //                     const Vector3& lightDir, double alpha) const {
    //     double NdotV = std::max(normal.dot(viewDir), 0.0);
    //     double NdotL = std::max(normal.dot(lightDir), 0.0);
    //     double ggx2 = geometrySchlickGGX(NdotV, alpha);
    //     double ggx1 = geometrySchlickGGX(NdotL, alpha);
    //
    //     return ggx1 * ggx2;
    // }
    //
    // /// Approximation de Fresnel par Schlick
    // Vector3 fresnelSchlick(double cosTheta, const Vector3& F0) const {
    //     double f = std::pow(1.0 - cosTheta, 5.0);
    //     return F0 + (Vector3(1.0, 1.0, 1.0) - F0) * f;
    // }
    //
    // /// Échantillonnage d'importance pour GGX
    // Vector3 sampleGGX(const Vector3& normal, double alpha, double u1, double u2) const {
    //     double theta = std::atan(alpha * std::sqrt(u1) / std::sqrt(1.0 - u1));
    //     double phi = 2.0 * M_PI * u2;
    //
    //     // Vecteur local (tangent space)
    //     Vector3 localH(
    //         std::sin(theta) * std::cos(phi),
    //         std::sin(theta) * std::sin(phi),
    //         std::cos(theta)
    //     );
    //
    //     // Transformation vers world space
    //     Vector3 up = std::abs(normal.z()) < 0.999 ? Vector3(0, 0, 1) : Vector3(1, 0, 0);
    //     Vector3 tangent = up.cross(normal).normalized();
    //     Vector3 bitangent = normal.cross(tangent);
    //
    //     return tangent * localH.x() + bitangent * localH.y() + normal * localH.z();
    // }


    // Matériaux prédéfinis mis à jour
    static const Material Mirror;
    static const Material Glass;
    static const Material MetalGold;
    static const Material MetalCopper;
    static const Material PlasticRough;
};