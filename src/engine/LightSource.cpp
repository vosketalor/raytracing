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
                         const float width,
                         const float height,
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

float LightSource::getIntensity() const {
    return intensity_;
}

GPU::GPULightSource LightSource::toGPU()
{
    GPU::GPULightSource data;
    data.position = glm::vec3(position_.x(), position_.y(), position_.z());
    data.colorDiffuse = glm::vec3(colorDiffuse_.x(), colorDiffuse_.y(), colorDiffuse_.z());
    data.colorSpecular = glm::vec3(colorSpecular_.x(), colorSpecular_.y(), colorSpecular_.z());
    data.intensity = intensity_;
    data.uDir = glm::vec3(uDir_.x(), uDir_.y(), uDir_.z());
    data.vDir = glm::vec3(vDir_.x(), vDir_.y(), vDir_.z());
    data.halfWidth = halfWidth_;
    data.halfHeight = halfHeight_;
    return data;
}

void LightSource::setIntensity(const float& intensity) {
    intensity_ = intensity;
}

// Vector3 LightSource::samplePointOnArea() const {
//     if (halfWidth_ <= 0.0 || halfHeight_ <= 0.0 ||
//         (uDir_.norm() == 0.0) || (vDir_.norm() == 0.0))
//     {
//         return position_;
//     }
//
//     const float rx = (static_cast<float>(rand()) / RAND_MAX) * 2.0 - 1.0;
//     const float ry = (static_cast<float>(rand()) / RAND_MAX) * 2.0 - 1.0;
//
//     const Vector3 offset = uDir_ * (rx * halfWidth_) + vDir_ * (ry * halfHeight_);
//     return position_ + offset;
// }
