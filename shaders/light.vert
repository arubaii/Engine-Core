#version 300 es
#ifdef GL_ES
precision highp float;
#endif

layout (location = 0) in vec3 position;

uniform mat4 u_MVP;

void main() {
    gl_Position = u_MVP * vec4(position, 1.0);
}