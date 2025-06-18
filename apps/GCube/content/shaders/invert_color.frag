#version 450

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D uInputImage;

void main() {
    vec4 color = texture(uInputImage, vUV);
    outColor = vec4(1.0 - color.rgb, color.a);
}