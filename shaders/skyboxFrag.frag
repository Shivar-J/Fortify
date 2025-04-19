#version 450

layout(set = 0, binding = 1) uniform samplerCube skybox;

layout(location = 0) in vec3 TexCoord;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(skybox, TexCoord);
}
