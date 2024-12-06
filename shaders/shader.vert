#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec2 fragUv;
layout (location = 3) out vec3 fragWorldPos;

layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 view;
	mat4 inverseView;
	mat4 projection;
	vec4 ambientLightColor;
	vec4 lightColor;
	vec3 directionalLight;
} ubo;

layout (push_constant) uniform Push {
	mat4 modelMatrix;
} push;

void main() {
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.projection * ubo.view * positionWorld;

	fragNormal = normalize(normalize(mat3(push.modelMatrix) * normal));
	fragWorldPos = positionWorld.xyz;
	fragColor = color;
	fragUv = uv;
}