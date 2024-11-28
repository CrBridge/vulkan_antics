#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUv;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 view;
	mat4 projection;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

layout (set = 1, binding = 1) uniform sampler2D texSampler;

layout (push_constant) uniform Push {
	mat4 modelMatrix;
} push;

void main() {
	vec3 directionToLight = ubo.lightPosition - fragPosWorld;
	float attenuation = 1.0 / dot(directionToLight, directionToLight);

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

	//outColor = texture(texSampler, fragUv);
	outColor = vec4((diffuseLight + ambientLight) * vec3(texture(texSampler, fragUv)), 1.0);
}