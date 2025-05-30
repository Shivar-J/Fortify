#version 450
#define MAX_LIGHTS 99

struct LightBuffer { 
	vec3 pos;
	float padding;
	vec3 color;
};

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 color;
	LightBuffer lights[MAX_LIGHTS];
	int numLights;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragTexColor;

void main() {
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
	fragColor = ubo.color;
	fragTexColor = inTexCoord;
	fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
}