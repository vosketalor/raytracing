#include <vector>
#include <memory>

class Shape;
class Light;

class Scene {
protected:
    std::vector<std::shared_ptr<Shape>> shapes;
    std::vector<std::shared_ptr<Light>> lights;

public:
    Scene() {}

    virtual ~Scene() = default;

    virtual void createShapes() = 0;
    virtual void createLights() = 0;

    const std::vector<std::shared_ptr<Shape>>& getShapes() const { return shapes; }
    const std::vector<std::shared_ptr<Light>>& getLights() const { return lights; }
};