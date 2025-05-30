#pragma once
#include <vector>
#include <memory>

#include "Vector.h"
#include "LightSource.h"
#include "shapes/Shape.h"

// Forward declarations
// class Shape;
// class LightSource;

class Scene {
protected:
    std::vector<std::shared_ptr<Shape>> shapes;
    std::vector<std::shared_ptr<LightSource>> lightSources;

    Vector3 ambient = {0.2, 0.2, 0.2};
    Vector3 skyColor = {0.0, 0.0, 0.0};
    Vector3 quadraticAttenuation = {1, 0.1, 0.01};

public:
    // ============== COLOR CONSTANTS ==============
    inline static constexpr Vector3 BLACK = Vector3(0, 0, 0);
    inline static constexpr Vector3 WHITE = Vector3(1, 1, 1);
    inline static constexpr Vector3 GRAY = Vector3(0.5, 0.5, 0.5);
    inline static constexpr Vector3 DARK_GRAY = Vector3(0.25, 0.25, 0.25);
    inline static constexpr Vector3 LIGHT_GRAY = Vector3(0.75, 0.75, 0.75);

    inline static constexpr Vector3 RED = Vector3(1, 0, 0);
    inline static constexpr Vector3 GREEN = Vector3(0, 1, 0);
    inline static constexpr Vector3 BLUE = Vector3(0, 0, 1);

    inline static constexpr Vector3 YELLOW = Vector3(1, 1, 0);
    inline static constexpr Vector3 CYAN = Vector3(0, 1, 1);
    inline static constexpr Vector3 MAGENTA = Vector3(1, 0, 1);

    inline static constexpr Vector3 ORANGE = Vector3(1, 0.5, 0);
    inline static constexpr Vector3 PURPLE = Vector3(0.5, 0, 0.5);
    inline static constexpr Vector3 PINK = Vector3(1, 0.5, 0.75);
    inline static constexpr Vector3 BROWN = Vector3(0.6, 0.3, 0.1);

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

    static constexpr double EPSILON = 1e-6;

    Scene() = default;
    virtual ~Scene() = default;

    virtual void createShapes() = 0;
    virtual void createLights() = 0;

    void addShape(const std::shared_ptr<Shape>& shape);
    void addLightSource(const std::shared_ptr<LightSource>& lightSource);

    void setSkyColor(const Vector3& color);
    void setAmbient(const Vector3& ambient);

    std::vector<std::shared_ptr<Shape>> getShapes() const { return shapes; }
    std::vector<std::shared_ptr<LightSource>> getLightSources() const { return lightSources; }
    const Vector3& getSkyColor() const { return skyColor; }
    const Vector3& getAmbient() const { return ambient; }
    const Vector3& getQuadraticAttenuation() const { return quadraticAttenuation; }
};