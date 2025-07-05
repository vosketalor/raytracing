#pragma once
#include "UIWindow.h"

class Application;

class InspectorWindow : public UIWindow {
public:
    InspectorWindow (Application& renderer);
    void renderShape(Application& renderer);
    void render() override;

private:
    Application& m_renderer;
};

