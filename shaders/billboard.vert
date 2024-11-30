#version 450

layout (location = 0) out vec2 fragOffset;

layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 view;
	mat4 projection;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

const vec2 offsets[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, 1.0)
);

const float RADIUS = 0.1;

void main () {
	//
	fragOffset = offsets[gl_VertexIndex];
	
	vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
	vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};
	// Would alternatively use this to fix the up vector, used for axial billboards
	//vec3 cameraUpWorld = vec3(0.0, 1.0, 0.0);
	vec3 positionWorld = ubo.lightPosition.xyz + RADIUS * fragOffset.x * cameraRightWorld + RADIUS * fragOffset.y * cameraUpWorld;

	gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}