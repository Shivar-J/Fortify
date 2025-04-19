#version 450

layout(set = 0, binding = 0) uniform SkyboxUBO {
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 TexCoord;

void main() {
    TexCoord = inPosition;
    mat4 viewWithOutTranslation = mat4(mat3(ubo.view));
	vec4 clipPos = ubo.proj * viewWithOutTranslation * vec4(inPosition, 1.0);
	gl_Position = clipPos.xyww;
}
