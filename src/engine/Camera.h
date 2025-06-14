#pragma once
#include "Vector.h"
#include <cmath>

#include "glm/vec3.hpp"
#include "glm/ext/quaternion_geometric.hpp"

#ifdef _WIN32
  #include <corecrt_math_defines.h>
#endif

class Camera
{
protected:
    glm::vec3 position;
    float pitch;
    float yaw;
    float roll;

    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    float movementSpeed;
    float mouseSensitivity;

public:
    explicit Camera(const glm::vec3& position = {0.f,0.f,0.f},
                    const float pitch = 0.f,
                    const float yaw = 0.f,
                    const float roll = 0.f,
                    const float fov = 60.f,
                    const float aspectRatio = 16.f / 9.f,
                    const float nearPlane = 0.1f,
                    const float farPlane = 1000.f)
    : position(position),
    pitch(pitch),
    yaw(yaw),
    roll(roll),
    fov(fov),
    aspectRatio(aspectRatio),
    nearPlane(nearPlane),
    farPlane(farPlane),
    movementSpeed(2.5f),
    mouseSensitivity(0.1f) {}

    ~Camera() = default;

    const glm::vec3& getPosition() const { return position; }
    float getPitch() const { return pitch; }
    float getYaw() const { return yaw; }
    float getRoll() const { return roll; }
    float getFov() const { return fov; }
    float getAspectRatio() const { return aspectRatio; }
    float getNearPlane() const { return nearPlane; }
    float getFarPlane() const { return farPlane; }

    void setPosition(const glm::vec3& pos) { position = pos; }
    void setPitch(const float p) { pitch = p; }
    void setYaw(const float y) { yaw = y; }
    void setRoll(const float r) { roll = r; }
    void setFov(const float f) { fov = f; }
    void setAspectRatio(const float a) { aspectRatio = a; }
    void setNearPlane(const float n) { nearPlane = n; }
    void setFarPlane(const float f) { farPlane = f; }

    glm::vec3 getDirection() const {
        const float pitchRad = pitch * static_cast<float>(M_PI) / 180.f;
        const float yawRad = yaw * static_cast<float>(M_PI) / 180.f;

        const float x = cos(pitchRad) * sin(yawRad);
        const float y = sin(pitchRad);
        const float z = -cos(pitchRad) * cos(yawRad);

        return glm::normalize(glm::vec3{x, y, z});
    }

    glm::vec3 getRight() const {
        const float yawRad = yaw * static_cast<float>(M_PI) / 180.f;
        const float rollRad = roll * static_cast<float>(M_PI) / 180.f;

        const glm::vec3 right{
            cos(yawRad) * cos(rollRad),
            -sin(rollRad),
            sin(yawRad) * cos(rollRad)
        };
        return glm::normalize(right);
    }

    glm::vec3 getUp() const {
        return glm::normalize(glm::cross(getRight(), getDirection()));
    }

    void moveForward(const float deltaTime) {
        position = position + getDirection() * movementSpeed * deltaTime;
    }

    void moveBackward(const float deltaTime) {
        position = position - getDirection() * movementSpeed * deltaTime;
    }

    void moveLeft(const float deltaTime) {
        position = position - getRight() * movementSpeed * deltaTime;
    }

    void moveRight(const float deltaTime) {
        position = position + getRight() * movementSpeed * deltaTime;
    }

    void moveUp(const float deltaTime) {
        position.y -= movementSpeed * deltaTime;
    }

    void moveDown(const float deltaTime) {
        position.y += movementSpeed * deltaTime;
    }

    void processMouseMovement(float xoffset, float yoffset, const bool constrainPitch = true) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (constrainPitch) {
            if (pitch > 89.0) pitch = 89.0;
            if (pitch < -89.0) pitch = -89.0;
        }
    }
};
