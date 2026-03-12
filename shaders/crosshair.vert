#version 300 es
#ifdef GL_ES
precision highp float;
#endif
layout (location = 0) in vec3 pos;

void main (){
    gl_Position = vec4(pos, 1.0);
}