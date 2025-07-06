#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoord;
};

struct RayPayload {
    vec3 color;
    vec3 attenuation;
    uint depth;
    uint rngState;
    uint instanceID;
    uint insideObj;
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
layout(set = 0, binding = 4) buffer Vertices { Vertex vertices[]; } vertexBuffers[];
layout(set = 0, binding = 5) buffer Indices { uint indices[]; } indexBuffers[];
layout(set = 0, binding = 8, std140) buffer InstanceTransforms { mat4 transforms[]; };

const float IOR = 1.5;
const vec3 glassTint = vec3(0.95, 0.98, 1.0);
const vec3 metalTints[4] = vec3[4](vec3(1.0, 0.71, 0.29), vec3(0.95, 0.64, 0.54), vec3(0.95), vec3(0.56, 0.57, 0.58));

float rand(inout uint state) {
    state = (1664525u * state + 1013904223u);
    return float(state & 0x00FFFFFF) / float(0x01000000);
}

void main() {
    uint primID = gl_PrimitiveID;
    uint instID = gl_InstanceCustomIndexEXT;
    bool isGlass = (instID % 2) == 0;

    mat4 transform = transforms[instID];

    uint i0 = indexBuffers[nonuniformEXT(instID)].indices[primID * 3 + 0];
    uint i1 = indexBuffers[nonuniformEXT(instID)].indices[primID * 3 + 1];
    uint i2 = indexBuffers[nonuniformEXT(instID)].indices[primID * 3 + 2];

    vec3 p0 = (transform * vec4(vertexBuffers[nonuniformEXT(instID)].vertices[i0].position, 1.0)).xyz;
    vec3 p1 = (transform * vec4(vertexBuffers[nonuniformEXT(instID)].vertices[i1].position, 1.0)).xyz;
    vec3 p2 = (transform * vec4(vertexBuffers[nonuniformEXT(instID)].vertices[i2].position, 1.0)).xyz;

    vec3 bary = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 hitPoint = bary.x * p0 + bary.y * p1 + bary.z * p2;

    mat3 normalMatrix = transpose(inverse(mat3(transform)));
    vec3 n0 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i0].normal);
    vec3 n1 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i1].normal);
    vec3 n2 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i2].normal);
    vec3 normal = normalize(bary.x * n0 + bary.y * n1 + bary.z * n2);

    vec3 viewDir = -gl_WorldRayDirectionEXT;
    vec3 newOrigin;
    vec3 newDir;

    if (payload.depth >= ubo.rayBounces) {
        payload.color = vec3(0);
        return;
    }

    if (payload.insideObj == 1 && payload.instanceID == instID && isGlass) {
        payload.insideObj = 0;
        vec3 forward = gl_WorldRayDirectionEXT;
        vec3 offset = forward * 0.001;
        payload.instanceID = instID;
        traceRayEXT(
            topLevelAS, 
            gl_RayFlagsSkipClosestHitShaderEXT, 
            0xFF, 0, 0, 0, 
            hitPoint + offset, 
            1e-9, forward, 
            1e9, 
            0
        );
        return;
    }

    if (isGlass) {
        float dotNV = dot(normal, viewDir);
        float eta = dotNV < 0.0 ? IOR : 1.0 / IOR;
        normal = dotNV < 0.0 ? -normal : normal;
        dotNV = abs(dotNV);

        vec3 T = refract(-viewDir, normal, eta);
        vec3 R = reflect(-viewDir, normal);
        bool TIR = length(T) == 0.0;

        float R0 = pow((1.0 - eta) / (1.0 + eta), 2.0);
        float fresnel = R0 + (1.0 - R0) * pow(1.0 - dotNV, 5.0);

        float r = rand(payload.rngState);
        bool reflect = r < fresnel || TIR;

        newDir = reflect ? R : T;
        newOrigin = hitPoint + 0.001 * newDir;

        payload.attenuation *= reflect ? vec3(1.0) : glassTint;
        payload.insideObj = reflect ? 0 : 1;
    } else {
        vec3 reflected = reflect(-viewDir, normal);
        newDir = reflected;
        newOrigin = hitPoint + 0.001 * newDir;
        payload.attenuation *= metalTints[instID % 4];
        payload.insideObj = 0;
    }

    payload.instanceID = instID;
    payload.depth++;
    traceRayEXT(
        topLevelAS, 
        gl_RayFlagsSkipClosestHitShaderEXT, 
        0xFF,
        0, 0, 0, 
        newOrigin, 
        1e-9, 
        newDir, 
        1e9, 
        0
    );
}