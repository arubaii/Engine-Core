#version 410 core

in vec2 v_UV;
out vec4 FragColor;

uniform sampler2D u_MaskTex;
uniform vec2 u_TexelSize;
uniform vec3 u_OutlineColor;
uniform int u_ThicknessPx;

void main()
{
    float center = texture(u_MaskTex, v_UV).r;

    // Only draw outline on background pixels
    if (center > 0.5)
    discard;

    int r = u_ThicknessPx;

    for (int y = -r; y <= r; y++)
    {
        for (int x = -r; x <= r; x++)
        {
            vec2 offset = vec2(x, y) * u_TexelSize;
            float neighbor = texture(u_MaskTex, v_UV + offset).r;

            // If any neighbor belongs to selected object,
            // we are at the silhouette edge
            if (neighbor > 0.5)
            {
                FragColor = vec4(u_OutlineColor, 1.0);
                return;
            }
        }
    }

    discard;
}