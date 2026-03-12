#pragma once

#define GL_SILENCE_DEPRECATION

#ifdef __EMSCRIPTEN__
	#include <GLES3/gl3.h>
	#define GLFW_INCLUDE_NONE
	#include <GLFW/glfw3.h>
#else
	#define GLFW_INCLUDE_NONE
	#include <glad/gl.h>
	#include <GLFW/glfw3.h>
#endif