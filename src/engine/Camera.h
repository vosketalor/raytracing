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
    double pitch;  // rotation autour de l'axe X (regarder en haut/bas)
    double yaw;    // rotation autour de l'axe Y (regarder à gauche/droite)
    double roll;   // rotation autour de l'axe Z (inclinaison de la tête)

    double fov;          // champ de vision en degrés
    double aspectRatio;  // largeur / hauteur
    double nearPlane;    // plan proche de clipping
    double farPlane;     // plan lointain de clipping

    double movementSpeed;
    double mouseSensitivity;

public:
    Camera(const Vector3& position = Vector3{0,0,0},
           const double pitch = 0.0,
           const double yaw = 0.0,
           const double roll = 0.0,
           const double fov = 60.0,
           const double aspectRatio = 16.0 / 9.0,
           const double nearPlane = 0.1,
           const double farPlane = 1000.0)
    : position(position), pitch(pitch), yaw(yaw), roll(roll),
      fov(fov), aspectRatio(aspectRatio), nearPlane(nearPlane), farPlane(farPlane),
      movementSpeed(2.5), mouseSensitivity(0.1) {}

    ~Camera() = default;

    const Vector3& getPosition() const { return position; }
    double getPitch() const { return pitch; }
    double getYaw() const { return yaw; }
    double getRoll() const { return roll; }
    double getFov() const { return fov; }
    double getAspectRatio() const { return aspectRatio; }
    double getNearPlane() const { return nearPlane; }
    double getFarPlane() const { return farPlane; }

    void setPosition(const Vector3& pos) { position = pos; }
    void setPitch(const double p) { pitch = p; }
    void setYaw(const double y) { yaw = y; }
    void setRoll(const double r) { roll = r; }
    void setFov(const double f) { fov = f; }
    void setAspectRatio(const double a) { aspectRatio = a; }
    void setNearPlane(const double n) { nearPlane = n; }
    void setFarPlane(const double f) { farPlane = f; }

    Vector3 getDirection() const {
        const double pitchRad = pitch * M_PI / 180.0;
        const double yawRad = yaw * M_PI / 180.0;

        const double x = cos(pitchRad) * sin(yawRad);
        const double y = sin(pitchRad);
        const double z = -cos(pitchRad) * cos(yawRad);

        return Vector3{x, y, z}.normalized();
    }

    Vector3 getRight() const {
        const double yawRad = yaw * M_PI / 180.0;
        const double rollRad = roll * M_PI / 180.0;

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

    void moveForward(double deltaTime) {
        position = position + getDirection() * movementSpeed * deltaTime;
    }

    void moveBackward(double deltaTime) {
        position = position - getDirection() * movementSpeed * deltaTime;
    }

    void moveLeft(double deltaTime) {
        position = position - getRight() * movementSpeed * deltaTime;
    }

    void moveRight(double deltaTime) {
        position = position + getRight() * movementSpeed * deltaTime;
    }

    void moveUp(double deltaTime) {
        position.y() += movementSpeed * deltaTime;
    }

    void moveDown(double deltaTime) {
        position.y() -= movementSpeed * deltaTime;
    }

    void processMouseMovement(double xoffset, double yoffset, bool constrainPitch = true) {
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
