#pragma once
#include "GLcommon.h"
#include <glm/vec2.hpp>
#include <iostream>
#include <glm/vec4.hpp>

#include "utils/SmartPtrs.h"


struct WindowProperties
{
	unsigned int Width = 0;
	unsigned int Height = 0;
	std::string Title   = "OpenGL";
	unsigned int MonitorSelected = 0;
	bool VSyncEnabled = false;

};


class Window
{
public:
    Window(const WindowProperties& props = WindowProperties());

    float GetDPIScale() const;

    ~Window();

    GLFWwindow* GetGLFWwindow() const { return m_Window; }

    void SetFBW(int fbw) { m_FramebufferWidth = fbw; }
    void SetFBH(int fbh) { m_FramebufferHeight = fbh; }
    int  GetFBW() const { return m_FramebufferWidth; }
    int  GetFBH() const { return m_FramebufferHeight; }

    // Currently used for: Raycasting, ImGui
    glm::vec2 GetLogicalViewport() const
    {
        int winW, winH;
        int fbW, fbH;

        glfwGetWindowSize(m_Window, &winW, &winH);
        glfwGetFramebufferSize(m_Window, &fbW, &fbH);

        float sx = float(fbW) / float(winW);
        float sy = float(fbH) / float(winH);

        return glm::vec2(
            float(m_RenderWidth)  / sx,
            float(m_RenderHeight) / sy
        );
    }

    glm::vec2 GetFramebufferViewport() const
    {
        return glm::vec2(
            float(m_RenderWidth),
            float(m_RenderHeight)
        );
    }

    int GetWindowWidth() const
    {
        int winW, winH;
        glfwGetWindowSize(m_Window, &winW, &winH);
        return winW;
    }

    int GetWindowHeight() const
    {
        int winW, winH;
        glfwGetWindowSize(m_Window, &winW, &winH);
        return winH;
    }

    glm::vec2 GetViewport() const
    {
        return glm::vec2((float)GetWindowWidth(), (float)GetWindowHeight());
    }

    float GetAspect() const
    {
        int h = GetWindowHeight();
        if (h == 0) return 1.0f;
        return (float)GetWindowWidth() / (float)h;
    }

    void SetInputMode(int mode, int value) const
    {
        glfwSetInputMode(m_Window, mode, value);
    }

    void SetCursorPosCallback(GLFWcursorposfun callback) const
    {
        glfwSetCursorPosCallback(m_Window, callback);
    }

    void AttachInput(class Input* input);

    void SetVSync(bool enabled = 0);

    static Scope<Window> Create
    (
        unsigned int width,
        unsigned int height,
        const std::string& title,
        unsigned int monitor_selected
    );

    bool ShouldClose() const;
    void SwapBuffers() const;
    void PollEvents()  const;

    void SetRenderRegion(float panelWidth, float bottomHeight)
    {


        float dpi = GetDPIScale();

        int fbW = m_FramebufferWidth;
        int fbH = m_FramebufferHeight;

        int panelWidthScaled   = int(panelWidth   * dpi);
        int bottomHeightScaled = int(bottomHeight * dpi);

        m_RenderX = panelWidthScaled;
        m_RenderY = bottomHeightScaled;
        m_RenderWidth  = fbW - m_RenderX;
        m_RenderHeight = fbH - m_RenderY;

        glViewport(m_RenderX, m_RenderY, m_RenderWidth, m_RenderHeight);
    }


    int GetRenderX() const { return m_RenderX; }
    int GetRenderY() const { return m_RenderY; }
    int GetRenderWidth()  const { return m_RenderWidth; }
    int GetRenderHeight() const { return m_RenderHeight; }

    float GetRenderAspect() const
    {
        if (m_RenderHeight == 0) return 1.0f;
        return (float)m_RenderWidth / (float)m_RenderHeight;
    }


private:
    GLFWwindow* m_Window = nullptr;
    WindowProperties m_WindowProperties;
    int m_FramebufferWidth;
    int m_FramebufferHeight;

    int m_RenderX = 0;
    int m_RenderY = 0;
    int m_RenderWidth  = 0;
    int m_RenderHeight = 0;
};