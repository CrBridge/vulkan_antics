#version 450

layout (location = 0) out vec3 fragDirection;

layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 view;
	mat4 inverseView;
	mat4 projection;
	vec4 ambientLightColor;
	vec4 lightColor;
	vec3 directionalLight;
} ubo;

vec4 positions[6] = vec4[](
    vec4(-1.0, -1.0, 1.0, 1.0),
    vec4( 1.0, -1.0, 1.0, 1.0),    
	vec4(-1.0,  1.0, 1.0, 1.0),
	vec4( 1.0,  -1.0, 1.0, 1.0),
    vec4( 1.0,  1.0, 1.0, 1.0),
	vec4(-1.0,  1.0, 1.0, 1.0)
);

void main() {
   gl_Position = positions[gl_VertexIndex];
   mat4 viewWithoutTranslation = mat4(mat3(ubo.view));
   vec4 worldDirection = inverse(ubo.projection * viewWithoutTranslation) * gl_Position;
   fragDirection = normalize(worldDirection.xyz / worldDirection.w);
}