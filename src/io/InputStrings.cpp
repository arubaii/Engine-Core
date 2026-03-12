#include "KeyCodes.h"
#include "MouseCodes.h"
#include "InputAction.h"
#include <algorithm>


Key::Code Key::StringToKey(const std::string& name)
	{
	    std::string s = name;
	    std::transform(s.begin(), s.end(), s.begin(), ::tolower);

	    if (s.size() == 1 && s[0] >= 'a' && s[0] <= 'z')
	        return static_cast<Key::Code>(GLFW_KEY_A + (s[0] - 'a'));

	    if (s.size() == 1 && s[0] >= '0' && s[0] <= '9')
	        return static_cast<Key::Code>(GLFW_KEY_0 + (s[0] - '0'));

	    if (s == "f1")  return Key::F1;
	    if (s == "f2")  return Key::F2;
	    if (s == "f3")  return Key::F3;
	    if (s == "f4")  return Key::F4;
	    if (s == "f5")  return Key::F5;
	    if (s == "f6")  return Key::F6;
	    if (s == "f7")  return Key::F7;
	    if (s == "f8")  return Key::F8;
	    if (s == "f9")  return Key::F9;
	    if (s == "f10") return Key::F10;
	    if (s == "f11") return Key::F11;
	    if (s == "f12") return Key::F12;

	    if (s == "space")     return Key::Space;
	    if (s == "enter")     return Key::Enter;
	    if (s == "tab")       return Key::Tab;
	    if (s == "backspace") return Key::Backspace;
	    if (s == "escape")    return Key::Escape;
	    if (s == "capslock")  return Key::CapsLock;

	    if (s == "leftshift")   return Key::LeftShift;
	    if (s == "rightshift")  return Key::RightShift;
	    if (s == "leftctrl")    return Key::LeftCtrl;
	    if (s == "rightctrl")   return Key::RightCtrl;
	    if (s == "leftalt")     return Key::LeftAlt;
	    if (s == "rightalt")    return Key::RightAlt;
	#if defined(__APPLE__)
		if (s == "leftcmd")     return Key::LeftCmd;
		if (s == "rightcmd")    return Key::RightCmd;
	#else
		if (s == "leftcmd" || s == "rightcmd")
			return Key::Invalid;
	#endif

	    if (s == "left")  return Key::LeftArrow;
	    if (s == "right") return Key::RightArrow;
	    if (s == "up")    return Key::UpArrow;
	    if (s == "down")  return Key::DownArrow;

	    if (s == "insert")   return Key::Insert;
	    if (s == "delete")   return Key::Delete;
	    if (s == "home")     return Key::Home;
	    if (s == "end")      return Key::End;
	    if (s == "pageup")   return Key::PageUp;
	    if (s == "pagedown") return Key::PageDown;

	    if (s == "apostrophe")   return Key::Apostrophe;
	    if (s == "comma")        return Key::Comma;
	    if (s == "minus")        return Key::Minus;
	    if (s == "period")       return Key::Period;
	    if (s == "slash")        return Key::Slash;
	    if (s == "semicolon")    return Key::Semicolon;
	    if (s == "equal")        return Key::Equal;
	    if (s == "leftbracket")  return Key::LeftBracket;
	    if (s == "rightbracket") return Key::RightBracket;
	    if (s == "backslash")    return Key::Backslash;

	    if (s == "kp0")        return Key::KP0;
	    if (s == "kp1")        return Key::KP1;
	    if (s == "kp2")        return Key::KP2;
	    if (s == "kp3")        return Key::KP3;
	    if (s == "kp4")        return Key::KP4;
	    if (s == "kp5")        return Key::KP5;
	    if (s == "kp6")        return Key::KP6;
	    if (s == "kp7")        return Key::KP7;
	    if (s == "kp8")        return Key::KP8;
	    if (s == "kp9")        return Key::KP9;
	    if (s == "kpdecimal")  return Key::KPDecimal;
	    if (s == "kpdivide")   return Key::KPDivide;
	    if (s == "kpmultiply") return Key::KPMultiply;
	    if (s == "kpsubtract") return Key::KPSubtract;
	    if (s == "kpadd")      return Key::KPAdd;
	    if (s == "kpenter")    return Key::KPEnter;
	    if (s == "kpequal")    return Key::KPEqual;

	    return static_cast<Key::Code>(0);
	}

Mouse::Code Mouse::StringToMouse(const std::string& name)
{
	std::string s = name;
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);

	if (s == "left")   return Mouse::Left;
	if (s == "right")  return Mouse::Right;
	if (s == "middle") return Mouse::Middle;

	if (s == "button0") return Mouse::Button0;
	if (s == "button1") return Mouse::Button1;
	if (s == "button2") return Mouse::Button2;
	if (s == "button3") return Mouse::Button3;
	if (s == "button4") return Mouse::Button4;
	if (s == "button5") return Mouse::Button5;
	if (s == "button6") return Mouse::Button6;
	if (s == "button7") return Mouse::Button7;

	return Mouse::Invalid;
}

Mouse::Axis StringToMouseAxis(const std::string& name)
{
	std::string s = name;
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);

	if (s == "up")    return Mouse::Axis::MouseWheelUp;
	if (s == "down")  return Mouse::Axis::MouseWheelDown;
	if (s == "left")  return Mouse::Axis::MouseWheelLeft;
	if (s == "right") return Mouse::Axis::MouseWheelRight;

	return static_cast<Mouse::Axis>(0);
}

InputAction StringToAction(const std::string& name)
{
	std::string s = name;
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);

	if (s == "primaryclick")   		return InputAction::PrimaryClick;
	if (s == "secondaryclick") 		return InputAction::SecondaryClick;
	if (s == "middleclick")    		return InputAction::MiddleClick;

	if (s == "moveforward")  		return InputAction::MoveForward;
	if (s == "movebackward") 		return InputAction::MoveBackward;
	if (s == "moveleft")     		return InputAction::MoveLeft;
	if (s == "moveright")    		return InputAction::MoveRight;
	if (s == "moveup")       		return InputAction::MoveUp;
	if (s == "movedown")     		return InputAction::MoveDown;

	if (s == "thirdpersonselect")   return InputAction::ThirdPersonSelect;

	if (s == "orbitselect")  		return InputAction::OrbitSelect;
	if (s == "orbitzoomin")  		return InputAction::OrbitZoomIn;
	if (s == "orbitzoomout") 		return InputAction::OrbitZoomOut;

	if (s == "entitydrag")   		return InputAction::EntityDrag;
	if (s == "entityselect") 		return InputAction::EntitySelect;

	if (s == "entitydelete")    	return InputAction::EntityDelete;
	if (s == "entityduplicate") 	return InputAction::EntityDuplicate;

	if (s == "undo") 				return InputAction::Undo;
	if (s == "redo") 				return InputAction::Redo;

	return static_cast<InputAction>(0);
}