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
	GLFWcursor* PointHand  		= glfwCreateStandardCursor(GLFW_HAND_CURSOR);
	GLFWcursor* Arrow 			= glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	GLFWcursor* GrabHand 		= LoadCursor("../assets/cursors/handgrabbing.png");	 // macos
	GLFWcursor* OpenHand 		= LoadCursor("../assets/cursors/handopen.png");		 // macos
	GLFWcursor* ResizeUpDown 	= LoadCursor("../assets/cursors/resizeupdown.png");	 // macos
	GLFWcursor* Move 			= LoadCursor("../assets/cursors/move.png");			 // macos
};
