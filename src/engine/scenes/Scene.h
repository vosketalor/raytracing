#pragma once
#include <vector>
#include <memory>

#include "Vector.h"
#include "LightSource.h"
#include "shapes/Shape.h"
#include "textures/TextureAtlas.h"

// Forward declarations
// class Shape;
// class LightSource;

class Scene {
protected:
    std::vector<std::shared_ptr<Shape>> shapes;
    std::vector<std::shared_ptr<LightSource>> lightSources;

    glm::vec3 ambient = {0.2f, 0.2f, 0.2f};
    glm::vec3 skyColor = {0.f, 0.f, 0.f};
    glm::vec3 quadraticAttenuation = {1.f, 0.1f, 0.01f};

public:
    // ============== COLOR CONSTANTS ==============
    static constexpr glm::vec3 BLACK = {0.f, 0.f, 0.f};
    static constexpr glm::vec3 WHITE = {1.f, 1.f, 1.f};
    static constexpr glm::vec3 GRAY = {0.5f, 0.5f, 0.5f};
    static constexpr glm::vec3 DARK_GRAY = {0.25f, 0.25f, 0.25f};
    static constexpr glm::vec3 LIGHT_GRAY = {0.75f, 0.75f, 0.75f};

    static constexpr glm::vec3 RED = {1.f, 0.f, 0.f};
    static constexpr glm::vec3 GREEN = {0.f, 1.f, 0.f};
    static constexpr glm::vec3 BLUE = {0.f, 0.f, 1.f};

    static constexpr glm::vec3 YELLOW = {1.f, 1.f, 0.f};
    static constexpr glm::vec3 CYAN = {0.f, 1.f, 1.f};
    static constexpr glm::vec3 MAGENTA = {1.f, 0.f, 1.f};

    static constexpr glm::vec3 ORANGE = {1, 0.5, 0};
    static constexpr glm::vec3 PURPLE = {0.5, 0, 0.5};
    static constexpr glm::vec3 PINK = {1, 0.5, 0.75};
    static constexpr glm::vec3 BROWN = {0.6, 0.3, 0.1};

    // ============== REFRACTION INDICES ==============
    static constexpr float ETA_AIR = 1.00029f;
    static constexpr float ETA_WATER = 1.333f;
    static constexpr float ETA_GLASS = 1.5f;
    static constexpr float ETA_CRYSTAL = 2.00f;
    static constexpr float ETA_DIAMOND = 2.417f;
    static constexpr float ETA_RUBY = 1.77f;
    static constexpr float ETA_PLEXIGLASS = 1.51f;
    static constexpr float ETA_EMERALD = 1.57f;
    static constexpr float ETA_CUBIC_ZIRCONIA = 2.1f;

    static constexpr float EPSILON = 1e-6f;
    static constexpr glm::vec3 WIREFRAME_COLOR = GREEN;
    static constexpr float WIREFRAME_THICKNESS = 0.025f;

    Scene() = default;
    virtual ~Scene() = default;

    virtual void createShapes() = 0;
    virtual void createLights() = 0;

    void addShape(const std::shared_ptr<Shape>& shape);
    void addLightSource(const std::shared_ptr<LightSource>& lightSource);

    void setSkyColor(const glm::vec3& color);
    void setAmbient(const glm::vec3& ambient);

    TextureAtlas texture_atlas;

    std::vector<std::shared_ptr<Shape>> getShapes() const { return shapes; }
    std::vector<std::shared_ptr<LightSource>> getLightSources() const { return lightSources; }
    const glm::vec3& getSkyColor() const { return skyColor; }
    const glm::vec3& getAmbient() const { return ambient; }
    const glm::vec3& getQuadraticAttenuation() const { return quadraticAttenuation; }
};