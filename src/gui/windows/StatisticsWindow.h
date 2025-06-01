#pragma once
#include "UIWindow.h"

class Application;

class StatisticsWindow : public UIWindow {
public:
    StatisticsWindow(Application& renderer);
    void render() override;

private:
    Application& m_renderer;
};