#version 450

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPosition;

void main() {
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	fragColor = inColor;
	fragTexCoord = inTexCoord;
	fragPosition = (ubo.model * vec4(inPosition, 1.0)).xyz;

	fragNormal = mat3(transpose(inverse(ubo.model))) * vec3(0.0, 0.0, 1.0);
}