#pragma once
#include <imgui.h>
#include <glm/glm.hpp>
#include "utils/SmartPtrs.h"
#include "../scene_core/ecs/Entity.h"
#include "core/Scene.h"



struct UI
{
    int fps;
    float frameTime;
    bool flightMode;
    glm::vec3 cameraPos;
    float pitch, yaw;
    float greyScale = 0.1f;
    bool showGrid = false;
    bool showAxes = true;
    bool showSkybox = false;
    bool showLights = false;
    bool showCrosshair = false;
    float geometryScale = 1.0f;

    bool dragAffectsVertical = true;
    bool dragAffectsXZ;

    bool enablePhysics = false;

};

namespace UIUtils
{
    inline bool  s_FlashingLights   = false;
    inline bool  s_LightsPrev       = false;
    inline float s_FlashElapsed     = 0.0f;
    inline float s_FlashToggleAccum = 0.0f;

    inline void StartLightsFlash(bool& showLights)
    {
        s_FlashingLights = true;
        s_LightsPrev = showLights;
        s_FlashElapsed = 0.0f;
        s_FlashToggleAccum = 0.0f;
        showLights = true;
    }

    inline void UpdateLightsFlash(float dt, bool& showLights)
    {
        if (!s_FlashingLights) return;

        s_FlashElapsed += dt;
        s_FlashToggleAccum += dt;

        if (s_FlashToggleAccum >= 0.6f)
        {
            int n = (int)(s_FlashToggleAccum / 0.6f);
            s_FlashToggleAccum -= 0.6f * (float)n;
            if (n % 2 == 1) showLights = !showLights;
        }

        if (s_FlashElapsed >= 10.0f)
        {
            s_FlashingLights = false;
            showLights = s_LightsPrev;
            s_FlashElapsed = 0.0f;
            s_FlashToggleAccum = 0.0f;
        }
    }
}


class UIPanel
{
public:
    static void Render(UI& data, entt::entity selected, Scene* scene);




    static const int s_MaxSamples;
    static float s_FPSHistory[120];
    static int   s_FPSOffset;
    static float s_RunningSum;
    static int   s_SampleCount;
    static float smoothedFPS;
};

