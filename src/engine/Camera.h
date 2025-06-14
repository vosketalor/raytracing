#pragma once
#include "Vector.h"
#include <cmath>

#ifdef _WIN32
  #include <corecrt_math_defines.h>
#endif

class Camera
{
protected:
    Vector3 position;
    float pitch;  // rotation autour de l'axe X (regarder en haut/bas)
    float yaw;    // rotation autour de l'axe Y (regarder à gauche/droite)
    float roll;   // rotation autour de l'axe Z (inclinaison de la tête)

    float fov;          // champ de vision en degrés
    float aspectRatio;  // largeur / hauteur
    float nearPlane;    // plan proche de clipping
    float farPlane;     // plan lointain de clipping

    float movementSpeed;
    float mouseSensitivity;

public:
    Camera(const Vector3& position = Vector3{0,0,0},
           const float pitch = 0.f,
           const float yaw = 0.f,
           const float roll = 0.f,
           const float fov = 60.f,
           const float aspectRatio = 16.f / 9.f,
           const float nearPlane = 0.1f,
           const float farPlane = 1000.f)
    : position(position), pitch(pitch), yaw(yaw), roll(roll),
      fov(fov), aspectRatio(aspectRatio), nearPlane(nearPlane), farPlane(farPlane),
      movementSpeed(2.5f), mouseSensitivity(0.1f) {}

    ~Camera() = default;

    const Vector3& getPosition() const { return position; }
    float getPitch() const { return pitch; }
    float getYaw() const { return yaw; }
    float getRoll() const { return roll; }
    float getFov() const { return fov; }
    float getAspectRatio() const { return aspectRatio; }
    float getNearPlane() const { return nearPlane; }
    float getFarPlane() const { return farPlane; }

    void setPosition(const Vector3& pos) { position = pos; }
    void setPitch(const float p) { pitch = p; }
    void setYaw(const float y) { yaw = y; }
    void setRoll(const float r) { roll = r; }
    void setFov(const float f) { fov = f; }
    void setAspectRatio(const float a) { aspectRatio = a; }
    void setNearPlane(const float n) { nearPlane = n; }
    void setFarPlane(const float f) { farPlane = f; }

    Vector3 getDirection() const {
        const float pitchRad = pitch * M_PI / 180.0;
        const float yawRad = yaw * M_PI / 180.0;

        const float x = cos(pitchRad) * sin(yawRad);
        const float y = sin(pitchRad);
        const float z = -cos(pitchRad) * cos(yawRad);

        return Vector3{x, y, z}.normalized();
    }

    Vector3 getRight() const {
        const float yawRad = yaw * M_PI / 180.0;
        const float rollRad = roll * M_PI / 180.0;

        const Vector3 right{
            cos(yawRad) * cos(rollRad),
            -sin(rollRad),
            sin(yawRad) * cos(rollRad)
        };
        return right.normalized();
    }

    Vector3 getUp() const {
        return getRight().cross(getDirection()).normalized();
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
        position.y() -= movementSpeed * deltaTime;
    }

    void moveDown(const float deltaTime) {
        position.y() += movementSpeed * deltaTime;
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
