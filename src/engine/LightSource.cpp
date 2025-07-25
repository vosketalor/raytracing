#include "LightSource.h"

LightSource::LightSource(const Vector3& position,
                         const Vector3& colorDiffuse,
                         const Vector3& colorSpecular)
    : position_(position),
      colorDiffuse_(colorDiffuse),
      colorSpecular_(colorSpecular),
      intensity_(1.0),
      uDir_(0,0,0),
      vDir_(0,0,0),
      halfWidth_(0),
      halfHeight_(0)
{}

LightSource::LightSource(const Vector3& position,
                         const Vector3& uDir,
                         const Vector3& vDir,
                         const double width,
                         const double height,
                         const Vector3& colorDiffuse,
                         const Vector3& colorSpecular)
    : position_(position),
      colorDiffuse_(colorDiffuse),
      colorSpecular_(colorSpecular),
      intensity_(1.0),
      uDir_(uDir.normalized()),
      vDir_(vDir.normalized()),
      halfWidth_(width  * 0.5),
      halfHeight_(height * 0.5)
{}

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

Vector3 LightSource::samplePointOnArea() const {
    if (halfWidth_ <= 0.0 || halfHeight_ <= 0.0 ||
        (uDir_.norm() == 0.0) || (vDir_.norm() == 0.0))
    {
        return position_;
    }

    const double rx = (static_cast<double>(rand()) / RAND_MAX) * 2.0 - 1.0;
    const double ry = (static_cast<double>(rand()) / RAND_MAX) * 2.0 - 1.0;

    const Vector3 offset = uDir_ * (rx * halfWidth_) + vDir_ * (ry * halfHeight_);
    return position_ + offset;
}
