#pragma once
#include "UIWindow.h"

class Application;

class ResolutionWindow : public UIWindow {
public:
    ResolutionWindow(Application& renderer);
    void render() override;

private:
    Application& m_renderer;
};