#version 300 es
#ifdef GL_ES
precision highp float;
#endif


in vec2 TexCoord;
in vec3 Dir;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform bool useNormalColors;
uniform bool useColor;
uniform bool useTexture2D;
uniform bool useTexture3D;

uniform vec4 u_Color;
uniform samplerCube u_Cubemap;
uniform sampler2D u_Texture;

uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{

    vec3 normal = normalize(Normal);

    vec3 lightDirection = normalize(lightPos - FragPos);
    float diff = max(dot(normal, lightDirection), 0.0);
    float lightIntensity = 1.2;
    float ambient = 0.0;

    float specularLight = 0.50;
    vec3 viewDirection = normalize(cameraPos - FragPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0), 8.0);
    float specular = specAmount * specularLight;

    vec4 baseColor = vec4(1.0, 0.0, 1.0, 1.0);
    if (useColor)
    baseColor = u_Color;

    if (useNormalColors)
    FragColor = vec4(normal * 0.5 + 0.5, 1.0);
    else
    FragColor = baseColor * lightColor * (diff + ambient + specular) * lightIntensity;
}