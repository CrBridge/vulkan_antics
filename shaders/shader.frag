#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec2 fragUv;
layout (location = 3) in vec3 fragWorldPos;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 view;
	mat4 inverseView;
	mat4 projection;
	vec4 ambientLightColor;
	vec4 lightColor;
	vec3 directionalLight;
} ubo;

layout (set = 1, binding = 2) uniform sampler2D texSampler;

layout (set = 1, binding = 3) uniform sampler2D terrainTex0;
layout (set = 1, binding = 4) uniform sampler2D terrainTex1;

layout (push_constant) uniform Push {
	mat4 modelMatrix;
} push;

vec4 calcTexColor()
{
	vec2 fragUvWrap = fragUv * 1000.0;
    vec4 texColor;
    float height = 1.0 * fragWorldPos.y;

	if (height < 10.0) {
		texColor = texture(terrainTex0, fragUvWrap);
	} else if (height < 30.0) {
		vec4 color0 = texture(terrainTex0, fragUvWrap);
		vec4 color1 = texture(terrainTex1, fragUvWrap);
		float factor = (height - 10.0) / (30.0 - 10.0);
		texColor = mix(color0, color1, factor);
	} else {
		texColor = texture(terrainTex1, fragUvWrap);
	}

    return texColor;
}

void main() {
	vec4 texColor = calcTexColor();
	vec3 normal = normalize(fragNormal);

	vec3 directionToLight = ubo.directionalLight;
	directionToLight = normalize(directionToLight);

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w;

	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(normal, directionToLight), 0);

	//outColor = vec4((diffuseLight + ambientLight) * vec3(texture(texSampler, fragUv)), 1.0);
	outColor = vec4((diffuseLight + ambientLight) * vec3(texColor), 1.0);
}