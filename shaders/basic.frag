#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;
out vec4 FragColor;

uniform vec3 uColor;
uniform vec3 uLightDir;
uniform int uUseLighting;

void main() {
    if (uUseLighting == 0) {
        FragColor = vec4(uColor, 1.0);
        return;
    }
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    float ndl = max(dot(N, L), 0.15);
    vec3 color = uColor * ndl;
    FragColor = vec4(color, 1.0);
}
