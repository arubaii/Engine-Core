#pragma once
#include <stb/stb_image.h>
#include "GLcommon.h"

inline static GLFWcursor* LoadCursor(const char* path)
{
	int w, h, channels;
	unsigned char* pixels = stbi_load(path, &w, &h, &channels, 4);
	if (!pixels)
		return nullptr;

	GLFWimage img;
	img.width  = w;
	img.height = h;
	img.pixels = pixels;

	// Center
	GLFWcursor* cursor = glfwCreateCursor(&img, w/2, h/2);

	stbi_image_free(pixels);
	return cursor;
}

struct EditorCursors
{
	GLFWcursor* PointHand;
	GLFWcursor* Arrow;
	GLFWcursor* GrabHand;        // macos
	GLFWcursor* OpenHand;        // macos
	GLFWcursor* ResizeUpDown;    // macos
	GLFWcursor* ResizeLeftRight; // macos
	GLFWcursor* ResizeDiagonal;  // macos
	GLFWcursor* Move;            // macos

	EditorCursors()
		: PointHand(       	glfwCreateStandardCursor(GLFW_HAND_CURSOR) ),
		  Arrow(           	glfwCreateStandardCursor(GLFW_ARROW_CURSOR) ),
		  GrabHand(        	LoadCursor("../assets/cursors/handgrabbing.png") ),     // macos
		  OpenHand(        	LoadCursor("../assets/cursors/handopen.png") ),         // macos
		  ResizeUpDown(		LoadCursor("../assets/cursors/resizeupdown.png") ),     // macos
		  ResizeLeftRight(	LoadCursor("../assets/cursors/resizeleftright.png") ),  // macos
		  ResizeDiagonal(   LoadCursor("../assets/cursors/resizediagonal.png") ),   // macos
		  Move(            	LoadCursor("../assets/cursors/move.png") )              // macos
	{}

	// For static initialization
	void Initialize()
	{
		PointHand       = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		Arrow           = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		GrabHand        = LoadCursor("../assets/cursors/handgrabbing.png");
		OpenHand        = LoadCursor("../assets/cursors/handopen.png");
		ResizeUpDown    = LoadCursor("../assets/cursors/resizeupdown.png");
		ResizeLeftRight = LoadCursor("../assets/cursors/resizeleftright.png");
		ResizeDiagonal  = LoadCursor("../assets/cursors/resizediagonal.png") ;
		Move            = LoadCursor("../assets/cursors/move.png");
	}
};
