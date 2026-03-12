#version 300 es
#ifdef GL_ES
precision highp float;
#endif

out vec3 WorldPos;

uniform mat4 Projection;
uniform mat4 View;

uniform vec3 CameraWorldPos;
uniform float gGridSize;
uniform float GridHeight;

const vec3 Quad[4] = vec3[4](
vec3(-1.0, 0.0, -1.0),
vec3( 1.0, 0.0, -1.0),
vec3( 1.0, 0.0,  1.0),
vec3(-1.0, 0.0,  1.0)
);

const uint Indices[6] = uint[6](0u,1u,2u, 0u,2u,3u);

void main()
{
    vec3 p = Quad[Indices[gl_VertexID]] * gGridSize;

    // center grid on origin you passed from C++
    p += CameraWorldPos;

    p.y = GridHeight;

    WorldPos = p;
    gl_Position = Projection * View * vec4(p, 1.0);
}