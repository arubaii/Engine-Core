#version 410 core
out vec4 FragColor;
uniform vec4 u_OutlineColor;

void main()
{
    FragColor = vec4(u_OutlineColor);
}
