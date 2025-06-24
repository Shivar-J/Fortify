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

float rand(inout uint state) {
    state = (1664525u * state + 1013904223u);
    return float(state & 0x00FFFFFF) / float(0x01000000);
}

const float IOR = 1.5;
const vec3 glassTint = vec3(0.95, 0.98, 1.0);

void main() {
    uint primID = gl_PrimitiveID;

    uint i0 = indices[primID * 3 + 0];
    uint i1 = indices[primID * 3 + 1];
    uint i2 = indices[primID * 3 + 2];

    Vertex v0 = vertices[i0];
    Vertex v1 = vertices[i1];
    Vertex v2 = vertices[i2];

    vec3 bary = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 P = bary.x * v0.position + bary.y * v1.position + bary.z * v2.position;
    vec3 N = normalize(bary.x * v0.normal + bary.y * v1.normal + bary.z * v2.normal);
    
    vec3 V = -gl_WorldRayDirectionEXT;
    float NdotV = dot(N, V);
    
    float eta, cosI;
    if (NdotV < 0.0) {
        N = -N;
        NdotV = -NdotV;
        eta = IOR;
    } else {
        eta = 1.0 / IOR;
    }
    
    vec3 R = reflect(-V, N);
    
    vec3 T = vec3(0.0);
    float fresnel = 1.0;
    bool tir = false;
    
    float sinT2 = eta * eta * (1.0 - NdotV * NdotV);
    if (sinT2 <= 1.0) {
        cosI = sqrt(1.0 - sinT2);
        T = refract(-V, N, eta);
        
        float R0 = pow((1.0 - eta) / (1.0 + eta), 2.0);
        fresnel = R0 + (1.0 - R0) * pow(1.0 - NdotV, 5.0);
    } else {
        tir = true;
    }
    
    float r = rand(payload.rngState);
    vec3 newDir;
    
    if (tir || r < fresnel) {
        newDir = R;
    } else {
        newDir = T;
        payload.attenuation *= glassTint;
    }
    
    vec3 newOrigin = P + sign(dot(newDir, N)) * N * 0.001;
    
    if (payload.depth >= ubo.rayBounces) {
        payload.color = vec3(0.0);
        return;
    }
    
    payload.depth++;
    traceRayEXT(
        topLevelAS,
        gl_RayFlagsNoneEXT,
        0xFF,
        0,
        0,
        0,
        newOrigin,
        0.01,
        newDir,
        1e9,
        0
    );
}
