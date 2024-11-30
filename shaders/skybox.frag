#version 450

layout (location = 0) in vec3 fragDirection;

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 2) uniform samplerCube skyboxSampler;

void main() {
    vec3 direction = normalize(fragDirection);
    outColor = texture(skyboxSampler, direction);
}