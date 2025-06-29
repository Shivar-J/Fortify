#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 color;
    vec3 attenuation;
    uint depth;
    uint rngState;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

layout(location = 0, binding = 6) uniform samplerCube skybox;

void main() {
    vec3 rayDir = normalize(gl_WorldRayDirectionEXT);

    payload.color = texture(skybox, rayDir).rgb;
}