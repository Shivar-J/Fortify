#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT RayPayload payload;

layout(set = 0, binding = 6) uniform samplerCube skybox;

void main() {
    vec3 rayDir = normalize(gl_WorldRayDirectionEXT);

    payload.color = texture(skybox, rayDir).rgb * payload.attenuation;
}