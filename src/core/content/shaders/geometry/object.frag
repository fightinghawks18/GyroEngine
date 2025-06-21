#version 450

layout(set = 1, binding = 0) uniform sampler2D uTexture;

layout(location = 0) in vec3 inFragPos;
layout(location = 1) in vec2 inVertexUV;
layout(location = 2) in vec4 inVertexCol;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(uTexture, inVertexUV) * inVertexCol;
}