#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragDirection;

layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionView;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

void main() {
    fragDirection = position;

    mat4 projectionViewWithoutTranslation = mat4(mat3(ubo.projectionView));
    gl_Position = projectionViewWithoutTranslation * vec4(position, 1.0);

    gl_Position.z = gl_Position.w;
}