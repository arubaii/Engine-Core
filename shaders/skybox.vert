#version 300 es
#ifdef GL_ES
precision highp float;
#endif
layout (location = 0) in vec3 aPos;

out vec3 vDir;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    vDir = aPos;

    vec4 pos = u_Projection * u_View * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}