#include "Window.h"
#include "io/Input.h"
#include "utils/Log.h"


#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif


Window::Window(const WindowProperties& props)
    : m_WindowProperties(props)
{
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

#ifdef __EMSCRIPTEN__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // REQUIRED on macOS
#endif

#ifdef __EMSCRIPTEN__
    int canvasW, canvasH;
    emscripten_get_canvas_element_size("#canvas", &canvasW, &canvasH);
    if (canvasW <= 0 || canvasH <= 0)
    {
        canvasW = 1280;
        canvasH = 720;
    }
    m_WindowProperties.Width  = canvasW;
    m_WindowProperties.Height = canvasH;
#else
    // Monitor and Window Size Query
    GLFWmonitor** monitors;
    int count;
    monitors = glfwGetMonitors(&count);

    if (!monitors || count == 0) {
        std::cerr << "ERROR: No monitors detected. Using default monitor 0.\n";
        m_WindowProperties.MonitorSelected = 0;
    }

    // Falls back to primary monitor rather than segfaulting
    if (m_WindowProperties.MonitorSelected >= static_cast<unsigned int>(count)) {
        std::cerr << "WARNING: Invalid monitor index (" << m_WindowProperties.MonitorSelected
                  << "). Falling back to primary monitor (0)." << std::endl;
        m_WindowProperties.MonitorSelected = 0;
    }

    GLFWmonitor* monitor = monitors[m_WindowProperties.MonitorSelected];
    int xpos, ypos, workW, workH;
    glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &workW, &workH);

    // Auto fallback: use full work area if width/height not specified
    if (m_WindowProperties.Width == 0 || m_WindowProperties.Height == 0) {
        m_WindowProperties.Width  = workW;
        m_WindowProperties.Height = workH;
    }
#endif

    // Window Creation
    m_Window = glfwCreateWindow(m_WindowProperties.Width,
                                m_WindowProperties.Height,
                                m_WindowProperties.Title.c_str(),
                                nullptr,
                                nullptr);
    if (!m_Window)
        throw std::runtime_error("Failed to create GLFW window");

#ifndef __EMSCRIPTEN__
    int windowX = xpos + (workW - m_WindowProperties.Width) / 2;
    int windowY = ypos + (workH - m_WindowProperties.Height) / 2;

    glfwSetWindowPos(m_Window, windowX, windowY); // Center of window
#endif

    glfwMakeContextCurrent(m_Window);

#ifndef __EMSCRIPTEN__
    if (!gladLoadGL(glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize OpenGL loader");
    }
#endif

    int fbw, fbh;
    glfwGetFramebufferSize(m_Window, &fbw, &fbh);

    m_FramebufferWidth  = fbw;
    m_FramebufferHeight = fbh;

#ifndef __EMSCRIPTEN__
    // Set initial viewport (Retina screens need framebuffer size)
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);

    // compute framebuffer size for this monitor
    int fbW = static_cast<int>(workW * xscale);
    int fbH = static_cast<int>(workH * yscale);

    glfwGetFramebufferSize(m_Window, &fbw, &fbh);
#endif

    // Resize callback
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window,
        [](GLFWwindow* win, int fbW, int fbH)
        {
            Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
            if (!self) return;

            self->m_FramebufferWidth  = fbW;
            self->m_FramebufferHeight = fbH;
        }
    );

    SetVSync(m_WindowProperties.VSyncEnabled);
}

// Some systems use a pixel ratio != 1 (HiDPI / DPI)
// e.g. macOS Retina renders at a 2x pixel ratio
float Window::GetDPIScale() const
{
    int winW, winH;
    int fbW, fbH;

    glfwGetWindowSize(m_Window, &winW, &winH);
    glfwGetFramebufferSize(m_Window, &fbW, &fbH);

    if (winW == 0 || winH == 0) return 1.0f;

    float sx = float(fbW) / float(winW);
    float sy = float(fbH) / float(winH);

    // Usually sx == sy, but take the average just in case
    return (sx + sy) * 0.5f;
}


Window::~Window()
{
    if (m_Window)
        glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::AttachInput(Input* input)
{
    input->Init(m_Window);
}

Scope<Window> Window::Create(unsigned int width,
                             unsigned int height,
                             const std::string& title,
                             unsigned int MonitorSelected)
{
    WindowProperties spec;
    spec.Width = width;
    spec.Height = height;
    spec.Title = title;
    spec.MonitorSelected = MonitorSelected;

    return CreateScope<Window>(spec);
}

bool Window::ShouldClose() const { return glfwWindowShouldClose(m_Window); }
void Window::SwapBuffers() const { glfwSwapBuffers(m_Window); }
void Window::PollEvents()  const { glfwPollEvents(); }

void Window::SetVSync(bool enabled)
{
    glfwSwapInterval(enabled);
}