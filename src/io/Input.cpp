#include <imgui.h>
#include <iostream>
#include "Input.h"
#include "utils/Log.h"
#include <yaml-cpp/yaml.h>

Input* Input::s_Instance = nullptr;
std::unordered_map<InputAction, std::vector<Binding>> Input::s_ActionBindings;
std::unordered_map<InputAction, bool> Input::s_ActionPrevState;

void Input::Init(GLFWwindow* window)
{
	m_Window = window;
	s_Instance = this;
	glfwSetCursorPosCallback(m_Window, MouseCallback);
	glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
	glfwSetScrollCallback(m_Window, ScrollCallback);
#ifdef __EMSCRIPTEN__
	Input::LoadBindings("config/controls.yaml");
#else
	Input::LoadBindings("../config/controls.yaml");
#endif

}

// GLFWwindow is incomplete, window must be passed through here
void Input::Update(GLFWwindow* window)
{

	if (IsKeyPressedOnce(Key::Tab) || s_Mouse.initCursor)
	{
		if (!s_Mouse.cursorEnabled)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

				// Center the cursor
				int w, h;
				glfwGetWindowSize(window, &w, &h);
				glfwSetCursorPos(window, w * 0.5, h * 0.5);
				s_Mouse.initCursor = false;
			}
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


		s_Mouse.cursorEnabled = !s_Mouse.cursorEnabled;
		s_Mouse.first = true;
	}

	// Update mouse state
	for (int b = 0; b <= GLFW_MOUSE_BUTTON_LAST; ++b)
	{
		bool isDown = glfwGetMouseButton(m_Window, b) == GLFW_PRESS;

		s_Mouse.pressedOnce[b]  =  isDown && !s_Mouse.down[b];
		s_Mouse.releasedOnce[b] = !isDown &&  s_Mouse.down[b];
		s_Mouse.down[b] = isDown;
	}

	// UI activity state
	ImGuiIO& io = ImGui::GetIO();

	// mouse-only blocking (hovering UI should not disable keyboard)
	m_IsInUI =
		io.WantCaptureMouse ||
		ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
		ImGui::IsAnyItemHovered() ||
		ImGui::IsAnyItemActive();
	if (s_Mouse.pressedOnce[GLFW_MOUSE_BUTTON_LEFT])
	{
		if (io.WantCaptureMouse)
		{
			m_IsInUI = true;
			s_KeyboardEnabled = false;
		}
		else
		{
			m_IsInUI = false;
			s_KeyboardEnabled = true;
		}
	}

}

bool Input::IsInUI() const
{
	return m_IsInUI;
}

bool Input::IsMouseCapturedByUI() const
{
	return ImGui::GetIO().WantCaptureMouse;
}

bool Input::IsKeyboardCapturedByUI() const
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

bool Input::IsKeyPressed(int key) const
{
	return glfwGetKey(m_Window, key) == GLFW_PRESS;
}

bool Input::IsKeyPressedOnce(int key)
{

	bool isDown  = IsKeyPressed(key);
	bool wasDown = m_LastKeyState[key];

	m_LastKeyState[key] = isDown;

	return isDown && !wasDown;
}

bool Input::IsKeyboardEnabled() const
{
	return s_KeyboardEnabled;
}

bool Input::IsMousePressed(int button) const
{
	if (m_IsInUI) return false;
	return s_Mouse.down[button];
}

bool Input::IsMousePressedOnce(int button)
{
	if (m_IsInUI) return false;
	return s_Mouse.pressedOnce[button];
}

glm::vec2 Input::GetMouseDelta()
{
	glm::vec2 delta{
		static_cast<float>(s_Mouse.dx),
		static_cast<float>(s_Mouse.dy)
	};

	s_Mouse.dx = 0.0;
	s_Mouse.dy = 0.0;

	return delta;
}

bool Input::IsCursorEnabled() const
{
	return s_Mouse.cursorEnabled;
}

void Input::MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (!s_Instance) return;

	auto& mouse = s_Instance->s_Mouse;

	if (mouse.first) {
		mouse.x = xpos;
		mouse.y = ypos;
		mouse.first = false;
		return;
	}

	mouse.dx = xpos - mouse.x;
	mouse.dy = mouse.y - ypos;
	mouse.x  = xpos;
	mouse.y  = ypos;

}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
		return;

	if (!s_Instance) return;

	auto& mouse = s_Instance->s_Mouse;

}

void Input::ScrollCallback(GLFWwindow*, double xoffset, double yoffset)
{
	s_Scroll.X = xoffset;  // Horizontal scroll
	s_Scroll.Y = yoffset;
}

glm::vec2 Input::GetGlobalMousePos(GLFWwindow* glfwWindow)
{
	glm::dvec2 mouseGLFW;
	glfwGetCursorPos(glfwWindow, &mouseGLFW.x, &mouseGLFW.y);
	return mouseGLFW;
}

glm::vec2 Input::GetViewportMousePos(const Window& window)
{
	double mx, my;
	glfwGetCursorPos(m_Window, &mx, &my);

	float rx = window.GetRenderX() / window.GetDPIScale();

	float vx = float(mx) - rx;
	float vy = float(my) ;

	return glm::vec2( vx, vy );
}

void Input::LoadBindings(const std::string& path)
{
    YAML::Node config = YAML::LoadFile(path);
    YAML::Node actions = config["actions"];

    for (auto it : actions)
    {
        std::string actionName = it.first.as<std::string>();
        InputAction actionEnum = StringToAction(actionName);

        for (auto bindingNode : it.second)
        {
            Binding b;
        	b.wasActive = false;

            if (bindingNode["key"])
            {
                b.type = BindingType::Key;
                b.key = Key::StringToKey(bindingNode["key"].as<std::string>());
            }

            else if (bindingNode["mouse"])
            {
                b.type = BindingType::Mouse;
                b.mouse = Mouse::StringToMouse(bindingNode["mouse"].as<std::string>());
            }

            else if (bindingNode["scroll"])
            {
                b.type = BindingType::Scroll;
                std::string dir = bindingNode["scroll"].as<std::string>();

                if (dir == "Up")   b.scrollDir = Binding::ScrollDir::Up;
                if (dir == "Down") b.scrollDir = Binding::ScrollDir::Down;
                if (dir == "X")    b.scrollDir = Binding::ScrollDir::X;
                if (dir == "Y")    b.scrollDir = Binding::ScrollDir::Y;
            }

            else if (bindingNode["chord"])
            {
            	b.type = BindingType::Chord;

            	for (auto k : bindingNode["chord"])
            	{
            		std::string item = k.as<std::string>();

            		Mouse::Code m = Mouse::StringToMouse(item);
            		if (m != Mouse::Invalid)
            		{
            			b.chord.push_back({false, Key::Code(0), m});
            			continue;
            		}

            		Key::Code key = Key::StringToKey(item);
            		b.chord.push_back({true, key, Mouse::Code(0)});
            	}
            }

            // Add to mapping
            s_ActionBindings[actionEnum].push_back(b);
        }
    }
}

bool Input::IsBindingActive(const Binding& b)
{
	switch (b.type)
	{
		case BindingType::Key:
			return IsKeyPressed(b.key);

		case BindingType::Mouse:
			return IsMousePressed(b.mouse);

		case BindingType::Scroll:
			if (b.scrollDir == Binding::ScrollDir::Up)   return s_Scroll.Y > 0;
			if (b.scrollDir == Binding::ScrollDir::Down) return s_Scroll.Y < 0;
			if (b.scrollDir == Binding::ScrollDir::X)    return s_Scroll.X != 0;
			if (b.scrollDir == Binding::ScrollDir::Y)    return s_Scroll.Y != 0;
			return false;

		case BindingType::Chord:
			for (auto& c : b.chord)
			{
				if (c.isKey)
				{
					if (!IsKeyPressed(c.key))
						return false;
				}
				else
				{
					if (!IsMousePressed(c.mouse))
						return false;
				}
			}
			return true;
	}
	return false;
}

bool Input::IsActionActive(InputAction a)
{
	auto& binds = s_ActionBindings[a];

	for (auto& b : binds)
		if (IsBindingActive(b))
			return true;

	return false;
}

bool Input::IsActionActiveOnce(InputAction a)
{
	auto& binds = s_ActionBindings[a];

	for (auto& b : binds)
	{
		bool active = IsBindingActive(b);

		if (active && !b.wasActive)
		{
			b.wasActive = true;
			return true;
		}

		b.wasActive = active;
	}

	return false;
}

void Input::EndFrame()
{
	s_Scroll = {};
}


