#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 color;
    vec3 attenuation;
    uint depth;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

void main() {
    payload.color = vec3(0.1, 0.1, 0.2);
}
