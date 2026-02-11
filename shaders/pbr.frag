#version 410 core

#define MAX_LIGHTS 32

struct SceneLight {
    vec3 Position;
    float Luminosity;   // Not used in shader (conversion done on CPU)
    float Intensity;    // Radiant intensity
    vec3 Color;
    float Radius;
};

uniform int u_LightCount;
uniform SceneLight u_Lights[MAX_LIGHTS];

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 UV;
    mat3 TBN;
} fs_in;

uniform vec3 u_CamPos;

uniform vec4 u_BaseColorFactor;
uniform float u_MetallicFactor;
uniform float u_RoughnessFactor;
uniform vec3 u_EmissiveFactor;
uniform float u_LightBoost;
uniform float u_Exposure;


uniform sampler2D u_BaseColorTex;
uniform sampler2D u_NormalTex;
uniform sampler2D u_MetallicRoughnessTex;
uniform sampler2D u_EmissiveTex;
uniform sampler2D u_OcclusionTex;
uniform samplerCube u_IrradianceMap;   // diffuse convolution of skybox
uniform samplerCube u_PrefilterMap;    // prefiltered specular mip chain
uniform sampler2D   u_BRDFLUT;         // 2D BRDF integration LUT

uniform samplerCube u_EnvMap;
uniform float u_MaxEnvMip;
uniform float u_EnvIntensity;
uniform float u_EmissiveStrength;


out vec4 FragColor;

const float PI = 3.14159265359;

// ============== BRDF helper functions ==============
vec3 F_Schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float D_GGX(float NdotH, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float G_Smith(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float g1 = NdotV / (NdotV * (1.0 - k) + k);
    float g2 = NdotL / (NdotL * (1.0 - k) + k);
    return g1 * g2;
}

vec3 ACESFilm(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x*(a*x+b)) / (x*(c*x+d)+e), 0.0, 1.0);
}

// === Normal mapping ===
vec3 GetNormal()
{
    vec3 tn = texture(u_NormalTex, fs_in.UV).xyz * 2.0 - 1.0;
    return normalize(fs_in.TBN * tn);
}

vec4 NormalMapColoring ()
{
    return vec4(normalize(fs_in.Normal) * 0.5 + 0.3, 1.0);
}

void main()
{
    // Base color
    vec3 baseTex = pow(texture(u_BaseColorTex, fs_in.UV).rgb, vec3(2.2));
    vec3 baseCol = baseTex * u_BaseColorFactor.rgb;

    // Metallic & roughness
    vec4 mr = texture(u_MetallicRoughnessTex, fs_in.UV);
    float roughness = clamp(mr.g * u_RoughnessFactor, 0.04, 1.0);
    float metallic  = mr.b * u_MetallicFactor;

    // Emissive
    vec3 emissive = pow(texture(u_EmissiveTex, fs_in.UV).rgb, vec3(2.2)) * u_EmissiveFactor;
    emissive *= u_EmissiveStrength;

    // Normal
    vec3 N = GetNormal();

    // View vector
    vec3 V = normalize(u_CamPos - fs_in.WorldPos);

    // Reflectance
    vec3 R = reflect(-V, N);
    float lod = roughness * u_MaxEnvMip;

    // === SKYBOX SPECULAR REFLECTION ===
    vec3 envSpec = textureLod(u_EnvMap, R, lod).rgb;

    // Fresnel base reflectance
    vec3 F0 = mix(vec3(0.04), baseCol, metallic);

    // Final outgoing radiance
    vec3 Lo = vec3(0.0);

    // === LIGHT LOOP ===
    for (int i = 0; i < u_LightCount; i++)
    {
        SceneLight Lgt = u_Lights[i];

        vec3 L = normalize(Lgt.Position - fs_in.WorldPos);
        vec3 H = normalize(V + L);

        float lightSizeScale = 3.0f;
        float R    = Lgt.Radius * lightSizeScale;
        float dist = length(Lgt.Position - fs_in.WorldPos);

        // Distance measured from the *surface* of the light
        float d = max(dist - R, 0.001);

        // Inverse-square falloff starting at the sphere surface
        float att = 1.0 / (d * d);

        // If the fragment is inside the light sphere, dampen the intensity smoothly
        if (dist < R)
        {
            att *= dist / R;   // prevents infinite brightness inside the light
        }

        // Radiance contribution
        vec3 radiance = Lgt.Intensity * Lgt.Color * att;
        float maxRadiance = 1500.0; // Tweak
        radiance = min(radiance, vec3(maxRadiance));

        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.001);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        // BRDF terms
        vec3 F = F_Schlick(HdotV, F0);
        float D = D_GGX(NdotH, roughness);
        float G = G_Smith(NdotV, NdotL, roughness);

        vec3 numerator = D * F * G;
        float denom = max(4.0 * NdotL * NdotV, 0.001);
        vec3 spec = numerator / denom;
        spec *= u_LightBoost; // Makes metals pop more

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

        vec3 diffuse = baseCol / PI;

        Lo += (kD * diffuse + spec) * radiance * NdotL ;
    }

    // Ambient occlusion
    float ao = texture(u_OcclusionTex, fs_in.UV).r;
    vec3 ambient = baseCol * (0.03 * ao);

    // Final color
    float NdotV = max(dot(N, V), 0.001);
    vec3 F_ibl = F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - NdotV, 5.0);
    vec3 kS_ibl = F_ibl;
    vec3 kD_ibl = (1.0 - kS_ibl) * (1.0 - metallic);


    vec3 diffuseIBL = vec3(0.0);
    vec3 specularIBL = envSpec * kS_ibl;

    diffuseIBL  *= u_EnvIntensity;
    specularIBL *= u_EnvIntensity;
    ambient = (kD_ibl * diffuseIBL + specularIBL) * ao;

    vec3 color = ambient + Lo + emissive;

    color *= u_Exposure;
    color = ACESFilm(color);
    color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}