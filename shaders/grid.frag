#version 410 core

in vec3 WorldPos;
out vec4 FragColor;

uniform float gGridCellSize;
uniform float gGridSize;
uniform float GridHeight;
uniform vec3 CameraWorldPos;

uniform vec4 gGridColorThin  = vec4(0.35, 0.35, 0.35, 1.0);
uniform vec4 gGridColorThick = vec4(0.7, 0.7, 0.7, 1.0);
uniform vec4 xAxisColor      = vec4(0.4, 0.8, 0.4, 1.0);
uniform vec4 zAxisColor      = vec4(0.9, 0.4, 0.4, 1.0);

void main()
{

    float distX = abs(WorldPos.x - CameraWorldPos.x);
    float distZ = abs(WorldPos.z - CameraWorldPos.z);

    float modX = mod(WorldPos.x - CameraWorldPos.x, gGridCellSize);
    float modZ = mod(WorldPos.z - CameraWorldPos.z, gGridCellSize);

    float lineX = 1.0 - min(modX, gGridCellSize - modX) / 0.02;
    float lineZ = 1.0 - min(modZ, gGridCellSize - modZ) / 0.02;

    float axisX = (abs(WorldPos.x - CameraWorldPos.x) < 0.03) ? 1.0 : 0.0;
    float axisZ = (abs(WorldPos.z - CameraWorldPos.z) < 0.03) ? 1.0 : 0.0;

    vec4 color =
    axisX * xAxisColor +
    axisZ * zAxisColor +
    (1.0 - axisX - axisZ) * max(lineX, lineZ) * gGridColorThin;

    FragColor = color;
}