#version 450

// Uniforms
layout(set = 1, binding = 0) uniform UBO
{
    mat4 model;       // Model matrix
    mat4 view;        // View matrix
    mat4 proj;        // Projection matrix
} ubo;

// I/O
layout(location = 0) in vec3 inVertexPos;       // Vertex position
layout(location = 1) in vec3 inVertexNor;       // Vertex normal
layout(location = 2) in vec2 inVertexUV;        // Vertex UV
layout(location = 3) in vec3 inVertexTangent;   // Vertex tangent
layout(location = 4) in vec4 inVertexCol;       // Vertex color

layout(location = 0) out vec3 outFragPos;       // Output fragment position
layout(location = 1) out vec2 outVertexUV;      // Output UV
layout(location = 2) out vec4 outVertexCol;     // Output vertex color

void main()
{
    vec3 worldPos = ubo.model * vec4(inVertexPos, 1.0);
    outFragPos = worldPos.xyz;

    // Transform vertex position to clip space
    gl_Position = ubo.proj * ubo.view * worldPos;

    // Pass variables to fragment shader
    outVertexCol = inVertexCol;
    outVertexUV = inVertexUV;
}