#pragma once
#include "UIWindow.h"

class Application;

class MenuBar : public UIWindow {
public:
    MenuBar(Application& renderer);
    void render() override;

private:
    Application& m_renderer;
};