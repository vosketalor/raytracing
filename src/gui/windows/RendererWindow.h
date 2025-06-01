#pragma once
#include "UIWindow.h"

class Application;

class RendererWindow : public UIWindow {
public:
    RendererWindow(Application& renderer);
    void render() override;

private:
    Application& m_renderer;
};