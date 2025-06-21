#version 450

// Structures
struct Light {
    int type; // 0: point, 1: directional, 2: spot
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float range; // Only for point lights
    float angle; // Only for spot lights
};

// Uniforms
layout(set = 0, binding = 1) uniform Camera {
    vec3 position;
    vec3 direction;
    vec3 up;
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
} cam; // Camera uniform for view and projection

// Textures
layout(set = 1, binding = 0) uniform sampler2D usTexture; // Object material texture

// Buffers
layout(set = 0, binding = 2) buffer LightBuffer {
    Light lights[];
} lightBuffer;

// PushConstants
layout(push_constant) uniform PushConstants {
    int lightCount;
} pushConstants;

// Inputs
layout(location = 0) in vec3 ivFragPosition; // Fragment position
layout(location = 1) in vec3 ivVertexNormal; // Vertex normal
layout(location = 2) in vec2 ivVertexUV; // Vertex uv
layout(location = 3) in vec4 ivVertexColor; // Vertex color

// Outputs
layout(location = 0) out vec4 outColor; // Final color output

// Helpers
float calculateSpecular(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 fragPosition, float shininess) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    return spec;
}

vec3 calculatePointLight(Light light, vec3 viewDir, vec3 fragPosition, vec3 normal) {
    vec3 lightDir = normalize(light.position - fragPosition);
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    float diffuse = max(dot(normal, lightDir), 0.0);
    float specular = calculateSpecular(lightDir, viewDir, normal, fragPosition, 32.0);
    return light.color * light.intensity * (diffuse + specular) * attenuation;
} // Calculates the light contribution for a point light

vec3 calculateDirectionalLight(Light light, vec3 viewDir, vec3 fragPosition, vec3 normal) {
    vec3 lightDir = normalize(-light.direction);
    float diffuse = max(dot(normal, lightDir), 0.0);
    float specular = calculateSpecular(lightDir, viewDir, normal, fragPosition, 32.0);
    return light.color * light.intensity * (diffuse + specular);
} // Calculates the light contribution for a directional light

vec3 calculateSpotLight(Light light, vec3 viewDir, vec3 fragPosition, vec3 normal) {
    vec3 lightDir = normalize(light.position - fragPosition);
    float angleCos = dot(lightDir, normalize(-light.direction));
    if (angleCos < cos(radians(light.angle))) {
        return vec3(0.0); // Outside the spot light cone
    }
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    float diffuse = max(dot(normal, lightDir), 0.0);
    float specular = calculateSpecular(lightDir, viewDir, normal, fragPosition, 32.0);
    return light.color * light.intensity * (diffuse + specular) * attenuation;
} // Calculates the light contribution for a spot light

void main() {
    // Calculate a viewing direction from the camera position to the fragment position
    vec3 viewDir = normalize(cam.position - ivFragPosition);
    vec3 lightResult = vec3(0.0); // This color dictates how much light hits this fragment

    // Go through each light in the LightBuffer and calculate its contribution
    for (int i = 0; i < pushConstants.lightCount; i++) {
        Light light = lightBuffer.lights[i];
        if (light.type == 1) { // Directional light
            lightResult += calculateDirectionalLight(light, viewDir, ivFragPosition, normalize(ivVertexNormal));
        } else if (light.type == 2) { // Spot light
            lightResult += calculateSpotLight(light, viewDir, ivFragPosition, normalize(ivVertexNormal));
        } else { // Point light or unrecognized type
            lightResult += calculatePointLight(light, viewDir, ivFragPosition, normalize(ivVertexNormal));
        }
    }

    vec4 texture = texture(usTexture, ivVertexUV); // Sample the texture
    vec4 finalColor = vec4(lightResult, 1.0) * texture * ivVertexColor; // Combine light result with texture and vertex color
    outColor = clamp(finalColor, 0.0, 1.0); // Output the final color and ensure it is within valid range
}