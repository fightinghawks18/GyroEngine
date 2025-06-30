#version 450

// Uniforms
layout(set = 0, binding = 1) uniform Camera {
    vec3 position;
    vec3 direction;
    vec3 up;
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
} cam;

// Textures
layout(set = 1, binding = 0) uniform sampler2D usTexture;

// Inputs
layout(location = 0) in vec3 ivFragPosition;
layout(location = 1) in vec3 ivVertexNormal;
layout(location = 2) in vec2 ivVertexUV;
layout(location = 3) in vec4 ivVertexColor;

// Outputs
layout(location = 0) out vec4 outColor;

void main() {
    vec4 textureColor = texture(usTexture, ivVertexUV);
    outColor = clamp(textureColor * ivVertexColor, 0.0, 1.0);
}