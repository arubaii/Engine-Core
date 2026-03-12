#version 300 es
#ifdef GL_ES
precision highp float;
#endif

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Color;
layout(location = 4) in vec3 a_Tangent;
layout(location = 5) in vec3 a_Bitangent;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec3 Dir;

uniform mat4 u_MVP;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_MVP * vec4(a_Position, 1.0);
    FragPos = vec3(u_Model * vec4(a_Position, 1.0));
    Normal = normalize(mat3(transpose(inverse(u_Model))) * a_Normal);

    Dir = normalize(mat3(u_Model) * a_Position);
    TexCoord = a_TexCoord;
}