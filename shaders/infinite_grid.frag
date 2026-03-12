#version 300 es
#ifdef GL_ES
precision highp float;
#endif

in vec3 WorldPos;

out vec4 FragColor;

uniform float gGridCellSize;
uniform float gGridMinPixelsBetweenCells;
uniform vec4 gGridColorThin;
uniform vec4 gGridColorThick;
uniform vec4 xAxisColor; // Green
uniform vec4 zAxisColor; // Red
uniform vec3 CameraWorldPos;
uniform float gGridSize;

uniform bool ShowAxes;

float log10(float x)
{
    return log(x) / log(10.0);
}

void main(){

    vec2 localXZ = WorldPos.xz - CameraWorldPos.xz;

    vec2 dvx = vec2(dFdx(localXZ.x), dFdy(localXZ.x));
    vec2 dvy = vec2(dFdx(localXZ.y), dFdy(localXZ.y));

    float lx = length(dvx);
    float ly = length(dvy);

    vec2 dudv = vec2(lx, ly);
    float l = length(dudv);
    float LOD = max(0.0, log10(l * gGridMinPixelsBetweenCells / gGridCellSize) + 1.0);

    float GridCellSizeLod0 = gGridCellSize * pow(10.0, floor(LOD));
    float GridCellSizeLod1 = GridCellSizeLod0 * 10.0;
    float GridCellSizeLod2 = GridCellSizeLod1 * 10.0;

    dudv *= 4.0;

    float xAxisWidth = dudv.y * 0.75;
    float zAxisWidth = dudv.x * 0.75;

    float xAxisMask = 1.0 - clamp(abs(localXZ.y) / xAxisWidth, 0.0, 1.0);
    float zAxisMask = 1.0 - clamp(abs(localXZ.x) / zAxisWidth, 0.0, 1.0);

    vec4 Color;

    if (xAxisMask > 0.0 && ShowAxes) {
        Color = xAxisColor;
        Color.a = xAxisMask;
    }
    else if (zAxisMask > 0.0 && ShowAxes) {
        Color = zAxisColor;
        Color.a = zAxisMask;
    }
    else
    {
        vec2 mod_div_dudv = mod(localXZ, GridCellSizeLod0) / dudv;
        float Lod0a = max(
        (1.0 - abs(clamp(mod_div_dudv.x, 0.0, 1.0) * 2.0 - 1.0)),
        (1.0 - abs(clamp(mod_div_dudv.y, 0.0, 1.0) * 2.0 - 1.0))
        );

        mod_div_dudv = mod(localXZ, GridCellSizeLod1) / dudv;
        float Lod1a = max(
        (1.0 - abs(clamp(mod_div_dudv.x, 0.0, 1.0) * 2.0 - 1.0)),
        (1.0 - abs(clamp(mod_div_dudv.y, 0.0, 1.0) * 2.0 - 1.0))
        );

        mod_div_dudv = mod(localXZ, GridCellSizeLod2) / dudv;
        float Lod2a = max(
        (1.0 - abs(clamp(mod_div_dudv.x, 0.0, 1.0) * 2.0 - 1.0)),
        (1.0 - abs(clamp(mod_div_dudv.y, 0.0, 1.0) * 2.0 - 1.0))
        );

        float LOD_fade = fract(LOD);

        if (Lod2a > 0.0)
        {
            Color = gGridColorThick;
            Color.a *= Lod2a;
        }
        else
        {
            if (Lod1a > 0.0)
            {
                Color = mix(gGridColorThick, gGridColorThin, LOD_fade);
                Color.a *= Lod1a;
            }
            else
            {
                Color = gGridColorThin;
                Color.a *= (Lod0a * (1.0 - LOD_fade));
            }
        }
    }

    float OpacityFalloff = 1.0 - clamp(length(localXZ) / gGridSize, 0.0, 1.0);

    Color.a *= OpacityFalloff;

    FragColor = Color;
}