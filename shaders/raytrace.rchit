#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable

struct RayPayload {
    vec3 color;
    vec3 attenuation;
    uint depth;
    uint rngState;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;
hitAttributeEXT vec2 attribs;

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;

layout(set = 0, binding = 3, std140) uniform RaytracingUBO {
    mat4 view;
    mat4 proj;
    uint vertexSize;
    uint sampleCount;
    uint samplesPerFrame;
    uint rayBounces;
} ubo;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoord;
};

layout(set = 0, binding = 4, scalar) buffer Vertices {
    Vertex vertices[];
};

layout(set = 0, binding = 5) buffer Indices {
    uint indices[];
};

uint rand_pcg(inout uint rngState) {
    uint state = rngState;
    rngState = rngState * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float rand(inout uint rngState) {
    return float(rand_pcg(rngState)) / 4294967295.0;
}

vec3 randomDirection(inout uint rngState) {
    float z = rand(rngState) * 2.0 - 1.0;
    float a = rand(rngState) * 6.283185307; // 2*pi
    float r = sqrt(1.0 - z*z);
    return vec3(r * cos(a), r * sin(a), z);
}

void main() {
    uint primID = gl_PrimitiveID;

    uint i0 = indices[primID * 3 + 0];
    uint i1 = indices[primID * 3 + 1];
    uint i2 = indices[primID * 3 + 2];

    Vertex v0 = vertices[i0];
    Vertex v1 = vertices[i1];
    Vertex v2 = vertices[i2];

    float u = attribs.x;
    float v = attribs.y;
    float w = 1.0 - u - v;

    vec3 normal = normalize(w * v0.normal + u * v1.normal + v * v2.normal);
    vec3 worldPos = w * v0.position + u * v1.position + v * v2.position;

    const float ior = 1.5;
    vec3 glassColor = vec3(0.95, 0.98, 1.0);

    vec3 rayDir = normalize(gl_WorldRayDirectionEXT);

    RayPayload savedPayload = payload;

    if (payload.depth < ubo.rayBounces) {
        vec3 reflectionDir = reflect(rayDir, normal);

        float eta = 1.0 / ior;
        vec3 refractionDir = refract(rayDir, normal, eta);

        float fresnel = 0.0;
        if (length(refractionDir) < 0.001) {
            fresnel = 1.0;
            refractionDir = reflectionDir;
        } else {
            float cosTheta = abs(dot(-rayDir, normal));
            float r0 = pow((1.0 - ior) / (1.0 + ior), 2.0);
            fresnel = r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);
        }

        payload.color = vec3(0);
        payload.attenuation = savedPayload.attenuation * glassColor;
        payload.depth = savedPayload.depth + 1;

        traceRayEXT(
            topLevelAS,
            gl_RayFlagsNoneEXT,
            0xFF,
            0, 0, 0,
            worldPos + normal * 0.001,
            0.001,
            reflectionDir,
            10000.0,
            0
        );

        vec3 reflectionColor = payload.color;

        payload = savedPayload;
        payload.color = vec3(0);
        payload.attenuation = savedPayload.attenuation * glassColor;
        payload.depth = savedPayload.depth + 1;

        traceRayEXT(
            topLevelAS,
            gl_RayFlagsNoneEXT,
            0xFF,
            0, 0, 0,
            worldPos - normal * 0.001,
            0.001,
            refractionDir,
            10000.0,
            0
        );

        vec3 refractionColor = payload.color;

        vec3 glassResult = mix(refractionColor, reflectionColor, fresnel);

        payload = savedPayload;
        payload.color = glassResult;
        payload.attenuation = savedPayload.attenuation;
        payload.depth = savedPayload.depth;
    } else {
        payload.color = vec3(0);
    }
}
