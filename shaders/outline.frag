#version 300 es
#ifdef GL_ES
precision highp float;
#endif
out vec4 FragColor;
uniform vec4 u_OutlineColor;

void main()
{
    FragColor = vec4(u_OutlineColor);
}