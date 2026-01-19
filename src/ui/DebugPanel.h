#pragma once
#include <imgui.h>
#include <glm/glm.hpp>
#include "utils/SmartPtrs.h"


struct DebugData
{
    int fps;
    float frameTime;
    bool flightMode;
    glm::vec3 cameraPos;
    float pitch, yaw;
    float greyScale = 0.1f;
    bool showGrid = true;

};


class DebugPanel
{
public:
    static void Render(DebugData& data);
};

