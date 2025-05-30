#pragma once
#include "Vector.h"

class Material {
protected:
    double reflectivity = 0.0;            ///< [0-1] reflection coefficient
    double transparency = 0.0;            ///< [0-1] transparency coefficient
    double shininess = 100.0;             ///< Phong shininess factor
    double eta = 1.0;                     ///< Refractive index

public:
    Material() = default;

    Material(const double refl, const double trans, const double shine, const double refractiveIndex)
        : reflectivity(refl), transparency(trans), shininess(shine), eta(refractiveIndex) {}

    virtual ~Material() = default;

    double getReflectivity() const { return reflectivity; }
    void setReflectivity(const double refl) { reflectivity = refl; }

    double getTransparency() const { return transparency; }
    void setTransparency(const double trans) { transparency = trans; }

    double getShininess() const { return shininess; }
    void setShininess(const double shine) { shininess = shine; }

    double getEta() const { return eta; }
    void setEta(const double refractiveIndex) { eta = refractiveIndex; }

    static const Material Mirror;
    static const Material Glass;
    static const Material Water;
    static const Material Metal;
    static const Material Wood;
    static const Material Plastic;
    static const Material Diamond;
};
