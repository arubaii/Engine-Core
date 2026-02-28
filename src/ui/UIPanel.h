#pragma once
#include <imgui.h>
#include <glm/glm.hpp>
#include "utils/SmartPtrs.h"
#include "../scene_core/ecs/Entity.h"
#include "core/Scene.h"



struct UIData
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
    bool dragAffectsXZ = false;

    bool enablePhysics = false;

    glm::vec2 viewportSize = { 0.0f, 0.0f };
    glm::vec2 framebufferSize = { 0.0f, 0.0f };
    GLuint sceneTextureID;

    bool splitterCursorHovered = false;

    glm::vec4 baseSkyColor{0.12f, 0.12f, 0.16f, 1.0f};
};

struct EditorConfig
{
    bool IsSplitterDraggingX;
    bool IsSplitterDraggingY;
    bool IsCornerDragging;
    bool IsCornerHovered;
};



class UIPanel
{
private:

public:
    static void Render(UIData& data, GLFWwindow* glfwWindow, Window* window, Scene* scene, Input* input, entt::entity selected);


    static void Splitter(UIData& data, EditorConfig& config,  GLFWwindow* glfwWindow, bool vertical, float thickness, float* size0, float* size1,
                         float min_size0, float min_size1, float totalWidth);

    static float GetPanelWidth()    { return s_UiWidth; }
    static float GetBottomHeight()  { return s_BottomHeight; }

    static void SetCursors(GLFWwindow* glfwWindow);

    static float s_UiWidth;
    static float s_BottomHeight;

    static const int s_MaxSamples;
    static float s_FPSHistory[120];
    static int   s_FPSOffset;
    static float s_RunningSum;
    static int   s_SampleCount;
    static float smoothedFPS;

    static EditorConfig s_Config;
    static EditorCursors s_Cursors;

    static ImVec2 s_CornerDragStartMouse;
    static float  s_CornerDragStartUiWidth;
    static float  s_CornerDragStartBottomHeight;
    static int s_CornerDragDominantAxis;
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


