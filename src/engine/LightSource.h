#pragma once
#include "GPULightSource.h"
#include "Vector.h"

class LightSource {
protected:
    Vector3 position_;        ///< Centre de l'area light (milieu du rectangle/disque)
    Vector3 colorDiffuse_;    ///< Couleur diffuse (RGB)
    Vector3 colorSpecular_;   ///< Couleur spéculaire (RGB)
    float intensity_;        ///< Intensité globale

    Vector3 uDir_, vDir_;     ///< Deux vecteurs unitaires formant la base du plan
    float halfWidth_, halfHeight_;  ///< Demi-largeur et demi-hauteur du rectangle

public:
    /**
     * @brief Constructeur pour light ponctuelle
     */
    LightSource(const Vector3& position,
                const Vector3& colorDiffuse,
                const Vector3& colorSpecular);

    /**
     * @brief Area light rectangulaire
     * @param position  Centre du rectangle
     * @param uDir      Direction unitaire d’un côté (longueur = 1)
     * @param vDir      Direction unitaire de l’autre côté (longueur = 1, perpendiculaire à uDir)
     * @param width     Largeur totale du rectangle
     * @param height    Hauteur totale du rectangle
     * @param colorDiffuse
     * @param colorSpecular
     */
    LightSource(const Vector3& position,
                const Vector3& uDir,
                const Vector3& vDir,
                float width,
                float height,
                const Vector3& colorDiffuse,
                const Vector3& colorSpecular);

    // Accesseurs classiques
    const Vector3& getPosition() const;
    const Vector3& getColorDiffuse() const;
    const Vector3& getColorSpecular() const;
    float getIntensity() const;
    GPU::GPULightSource toGPU();

    void setIntensity(const float& intensity);

    // Vector3 samplePointOnArea() const;
};
