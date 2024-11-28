#version 450

layout (location = 0) in vec3 fragDirection;

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 2) uniform samplerCube skyboxSampler;

void main() {
    // Use the normalized direction to sample the cubemap
    vec3 direction = normalize(fragDirection);
    outColor = texture(skyboxSampler, direction);
    //outColor = vec4(1.0, 1.0, 1.0, 1.0);
}