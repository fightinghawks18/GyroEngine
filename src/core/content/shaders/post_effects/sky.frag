#version 450

// Uniforms
layout(set = 0, binding = 0) uniform samplerCube uSkyTex; // Skybox texture
layout(set = 0, binding = 1) uniform UBO {
    mat4 uViewProjInv; // Inverse view-projection matrix
};

// I/O
layout(location = 0) in vec2 inUV; // Fullscreen Quad UV coordinates
layout(location = 0) out vec4 outColor; // Output color

void main() {
    // Convert UV [0,1] to NDC [-1,1]
    vec2 ndc = inUV * 2.0 - 1.0;
    // Fullscreen quad is at z = 1 (far plane), w = 1
    vec4 clip = vec4(ndc, 1.0, 1.0);

    // Transform to world space using inverse view-projection
    vec4 world = uViewProjInv * clip;
    vec3 direction = normalize(world.xyz / world.w);

    outColor = texture(uSkyTex, direction);
}