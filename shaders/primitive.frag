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

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	float ambientStrength = 0.1;
	vec3 norm = normalize(-fragNormal);

	vec3 ambient = vec3(0.0);
	vec3 diffuse = vec3(0.0);

	for (int i = 0; i < ubo.numLights; i++) {
		ambient += ambientStrength * ubo.lights[i].color;

		vec3 lightDir = normalize(ubo.lights[i].pos - fragPos);
		float diff = dot(norm, lightDir);

		diffuse += diff * ubo.lights[i].color;
	}

	vec3 result = (ambient + diffuse) * fragColor;
	outColor = vec4(result, 1.0);
}