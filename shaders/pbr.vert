#version 410 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Color;
layout(location = 4) in vec3 a_Tangent;
layout(location = 5) in vec3 a_Bitangent;

uniform mat4 u_MVP;
uniform mat4 u_Model;

out VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 UV;
    mat3 TBN;
} vs_out;

void main()
{
    vec4 wp = u_Model * vec4(a_Position, 1.0);
    vs_out.WorldPos = wp.xyz;

    mat3 Nmat = transpose(inverse(mat3(u_Model)));

    vec3 N = normalize(Nmat * a_Normal);


    mat3 M = mat3(u_Model);
    vec3 T = normalize(M * a_Tangent);
    vec3 B = normalize(M * a_Bitangent);

    // If tangents are garbage/zero, build a fallback basis
    if (length(T) < 1e-6) {
        vec3 up = (abs(N.y) < 0.999) ? vec3(0,1,0) : vec3(1,0,0);
        T = normalize(cross(up, N));
        B = normalize(cross(N, T));
    } else {
        T = normalize(T - N * dot(T, N));
        float handedness = (dot(cross(N, T), B) < 0.0) ? -1.0 : 1.0;
        B = normalize(cross(N, T)) * handedness;
    }

    vs_out.Normal = N;
    vs_out.TBN    = mat3(T, B, N);
    vs_out.UV     = a_TexCoord;

    gl_Position = u_MVP * vec4(a_Position, 1.0);
}
