#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 color;
    vec3 attenuation;
    uint depth;
    uint rngState;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

void main() {
    vec3 rayDir = normalize(gl_WorldRayDirectionEXT);
    float t = 0.5 * (rayDir.y + 1.0);
    vec3 skyColor = mix(vec3(0.5, 0.7, 1.0), vec3(0.1, 0.1, 0.3), t);

    payload.color = skyColor;
}
