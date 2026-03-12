#pragma once
#include <string>

enum class InputAction
{
	PrimaryClick,
	SecondaryClick,
	MiddleClick,

	MoveForward,
	MoveBackward,
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,

	ThirdPersonSelect,

	OrbitSelect,
	OrbitZoomIn,
	OrbitZoomOut,

	EntityDrag,
	EntitySelect,

	EntityDelete,
	EntityDuplicate,

	Undo,
	Redo
};

InputAction StringToAction(const std::string& name);