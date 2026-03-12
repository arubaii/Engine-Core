#version 300 es
#ifdef GL_ES
precision highp float;
#endif

out float FragMask;

void main()
{
    FragMask = 1.0;
}