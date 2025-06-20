#version 450

struct Light {
    int type; // 0: point, 1: directional, 2: spot
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float radius;
    float angle; // for spot lights
    float padding0;
}

layout(set = 2, binding = 0) uniform sampler2D uTexture;

layout(std430, set = 1, binding = 0) buffer LightBuffer {
    Light lights[];
} lights;
uniform int lightCount;

layout(set = 0, binding = 0) uniform Camera {
    vec3 position;
} camera;

layout(location = 0) in vec3 inFragPos;
layout(location = 1) in vec2 inVertexUV;
layout(location = 2) in vec4 inVertexCol;

layout(location = 0) out vec4 outColor;


// Calcu
vec3 computePointLight(Light light, vec3 pos, vec3 nor, vec3 view) {
    vec3 lightDir = normalize(light.position - pos);
    float diff = max(dot(nor, lightDir), 0.0);

    return light.color * diff * light.intensity;
}

void main() {
    vec3 view = normalize(camera.position - inFragPos);
    vec3 result = vec3(0.0);

    for (int i = 0; i < lightCount; ++i) {

    }

    outColor = texture(uTexture, inVertexUV) * inVertexCol;
}