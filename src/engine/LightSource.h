#pragma once
#include "Vector.h"

/**
 * @class LightSource
 * @brief Represents a point light source in 3D space for ray tracing illumination
 *
 * A light source emits light from a specific position with configurable
 * diffuse and specular components, and includes an intensity parameter
 * to control overall brightness.
 */
class LightSource {
protected:
    Vector3 position_;        ///< World-space position of the light
    Vector3 colorDiffuse_;    ///< Diffuse color component (RGB)
    Vector3 colorSpecular_;   ///< Specular color component (RGB)
    double intensity_;       ///< Light intensity multiplier (default: 1.0)

public:
    /**
     * @brief Constructs a new light source
     * @param position The world-space position
     * @param colorDiffuse The diffuse color (RGB)
     * @param colorSpecular The specular color (RGB)
     */
    LightSource(const Vector3& position,
               const Vector3& colorDiffuse,
               const Vector3& colorSpecular);

    // Accessors
    const Vector3& getPosition() const;
    const Vector3& getColorDiffuse() const;
    const Vector3& getColorSpecular() const;
    double getIntensity() const;

    // Mutators
    void setIntensity(const double& intensity);
};