#version 300 es
#ifdef GL_ES
precision highp float;
#endif
in vec3 vDir;
out vec4 FragColor;

uniform samplerCube u_Skybox;
uniform float u_Exposure;

void main()
{
    vec3 c = texture(u_Skybox, vDir).rgb;

    // exposure
    c = vec3(1.0) - exp(-c * u_Exposure);
    // gamma
    c = pow(c, vec3(1.0/2.2));

    FragColor = vec4(c, 1.0);
}