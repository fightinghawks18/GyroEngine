#version 450

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D uImage;
layout(push_constant) uniform PushConstants {
    float texelSize; // 1.0 / texture height
};

void main() {
    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    vec3 color = texture(uImage, vUV).rgb * weights[0];
    for (int i = 1; i < 5; ++i) {
        color += texture(uImage, vUV + vec2(0.0, texelSize * i)).rgb * weights[i];
        color += texture(uImage, vUV - vec2(0.0, texelSize * i)).rgb * weights[i];
    }
    outColor = vec4(color, 1.0);
}