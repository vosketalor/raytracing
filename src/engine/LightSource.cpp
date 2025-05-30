#include "LightSource.h"

LightSource::LightSource(const Vector3& position,
                        const Vector3& colorDiffuse,
                        const Vector3& colorSpecular)
    : position_(position),
      colorDiffuse_(colorDiffuse),
      colorSpecular_(colorSpecular),
      intensity_(1.0) {}

const Vector3& LightSource::getPosition() const {
    return position_;
}

const Vector3& LightSource::getColorDiffuse() const {
    return colorDiffuse_;
}

const Vector3& LightSource::getColorSpecular() const {
    return colorSpecular_;
}

double LightSource::getIntensity() const {
    return intensity_;
}

void LightSource::setIntensity(const double& intensity) {
    intensity_ = intensity;
}