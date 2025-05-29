#pragma once
#include <vector>
#include <memory>
#include "Vector.h"

// Forward declarations
class Shape;
class Light;

class Scene {
protected:
    std::vector<std::shared_ptr<Shape>> shapes;
    std::vector<std::shared_ptr<Light>> lights;

    Vector3 ambient = {0.2, 0.2, 0.2};
    Vector3 skyColor = {0.0, 0.0, 0.0};
    Vector3 quadraticAttenuation = {1, 0.1, 0.01};

public:
    // ============== COLOR CONSTANTS ==============
    static const Vector3 BLACK;
    static const Vector3 WHITE;
    static const Vector3 GRAY;
    static const Vector3 DARK_GRAY;
    static const Vector3 LIGHT_GRAY;

    static const Vector3 RED;
    static const Vector3 GREEN;
    static const Vector3 BLUE;

    static const Vector3 YELLOW;
    static const Vector3 CYAN;
    static const Vector3 MAGENTA;

    static const Vector3 ORANGE;
    static const Vector3 PURPLE;
    static const Vector3 PINK;
    static const Vector3 BROWN;

    // ============== REFRACTION INDICES ==============
    static constexpr double ETA_AIR = 1.00029;
    static constexpr double ETA_WATER = 1.333;
    static constexpr double ETA_GLASS = 1.5;
    static constexpr double ETA_CRYSTAL = 2.00;
    static constexpr double ETA_DIAMOND = 2.417;
    static constexpr double ETA_RUBY = 1.77;
    static constexpr double ETA_PLEXIGLASS = 1.51;
    static constexpr double ETA_EMERALD = 1.57;
    static constexpr double ETA_CUBIC_ZIRCONIA = 2.1;

    Scene();
    virtual ~Scene() = default;

    virtual void createShapes() = 0;
    virtual void createLights() = 0;

    const std::vector<std::shared_ptr<Shape>>& getShapes() const { return shapes; }
    const std::vector<std::shared_ptr<Light>>& getLights() const { return lights; }
};