#pragma once
#include "UIWindow.h"

class Application;

class MainViewport : public UIWindow {
public:
    MainViewport(Application& renderer);
    void render() override;

private:
    Application& m_renderer;
};