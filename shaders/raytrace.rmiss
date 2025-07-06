#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

struct RayPayload {
    vec3 color;
    vec3 attenuation;
    uint depth;
    uint rngState;
    uint instanceID;
    uint insideObj;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

layout(set = 0, binding = 6) uniform samplerCube skybox;

void main() {
    vec3 rayDir = normalize(gl_WorldRayDirectionEXT);

    payload.color = texture(skybox, rayDir).rgb * payload.attenuation;
}