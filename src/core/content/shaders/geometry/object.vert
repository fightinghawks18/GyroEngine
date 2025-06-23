#version 450

// Uniforms
layout(set = 0, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp; // MVP are the matrices needed to transform a vertex in 3D (or 2D if projection is identity) space

// Inputs
layout(location = 0) in vec3 ivVertexPosition;
layout(location = 1) in vec3 ivVertexNormal;
layout(location = 2) in vec2 ivVertexUV;
layout(location = 3) in vec3 ivVertexTangent;
layout(location = 4) in vec4 ivVertexColor;

// Outputs
layout(location = 0) out vec3 ovFragPosition; // Fragment position
layout(location = 1) out vec3 ovVertexNormal; // Vertex normal
layout(location = 2) out vec2 ovVertexUV; // Vertex uv
layout(location = 3) out vec4 ovVertexColor; // Vertex color

// Helpers
vec3 getWorldPosition() {
    return (mvp.model * vec4(ivVertexPosition, 1.0)).xyz;
} // Returns the world position of the vertex

vec4 getProjectedVertexPosition(vec3 worldPosition) {
    return mvp.projection * mvp.view * vec4(worldPosition, 1.0);
} // Returns the projected position of the vertex

void main() {
    // Get world position of the vertex
    vec3 worldPosition = getWorldPosition();
    vec3 vertexNormal = normalize((mvp.model * vec4(ivVertexNormal, 0.0)).xyz);

    // Transform the vertex position to clip space
    gl_Position = getProjectedVertexPosition(worldPosition);

    // Pass the attributes to the fragment shader
    ovFragPosition = worldPosition;
    ovVertexNormal = vertexNormal;
    ovVertexUV = ivVertexUV;
    ovVertexColor = ivVertexColor;
}