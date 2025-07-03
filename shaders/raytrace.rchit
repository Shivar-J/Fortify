#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable

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

layout(set = 0, binding = 4, scalar) buffer Vertices {
    Vertex vertices[];
};

layout(set = 0, binding = 5) buffer Indices {
    uint indices[];
};

layout(set = 0, binding = 8, std140) buffer InstanceTransforms {
    mat4 transforms[];
};

float rand(inout uint state) {
    state = (1664525u * state + 1013904223u);
    return float(state & 0x00FFFFFF) / float(0x01000000);
}

const float IOR = 1.5;
const vec3 glassTint = vec3(0.95, 0.98, 1.0);

void main() {
    uint primID = gl_PrimitiveID;
    uint instID = gl_InstanceCustomIndexEXT;

    mat4 transform = transforms[instID];

    uint i0 = indices[primID * 3 + 0];
    uint i1 = indices[primID * 3 + 1];
    uint i2 = indices[primID * 3 + 2];

    vec4 worldPos0 = transform * vec4(vertices[i0].position, 1.0);
    vec4 worldPos1 = transform * vec4(vertices[i1].position, 1.0);
    vec4 worldPos2 = transform * vec4(vertices[i2].position, 1.0);

    vec3 bary = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 worldPos = bary.x * worldPos0.xyz + bary.y * worldPos1.xyz + bary.z * worldPos2.xyz;
    mat3 normalMatrix = transpose(inverse(mat3(transform)));

    vec3 n0 = normalize(normalMatrix * vertices[i0].normal);
    vec3 n1 = normalize(normalMatrix * vertices[i1].normal);
    vec3 n2 = normalize(normalMatrix * vertices[i2].normal);

    vec3 worldNormal = normalize(bary.x * n0 + bary.y * n1 + bary.z * n2);

    vec3 viewDir = -gl_WorldRayDirectionEXT;
    
    float normalDotView = dot(worldNormal, viewDir);

    float eta, cosI;
    
    if(normalDotView < 0.0) {
        worldNormal = -worldNormal;
        normalDotView = -normalDotView;
        eta = IOR;
    } else {
        eta = 1.0 / IOR;
    }

    vec3 reflection = reflect(-viewDir, worldNormal);

    vec3 T = vec3(0.0);
    float fresnel = 1.0f;
    bool totalInternalReflection = false;

    float sinT2 = eta * eta * (1.0 - normalDotView * normalDotView);

    if(sinT2 <= 1.0) {
        cosI = sqrt(1.0 - sinT2);
        T = refract(-viewDir, worldNormal, eta);

        float R0 = pow((1.0 - eta) / (1.0 + eta), 2.0);
        fresnel = R0 + (1.0 - R0) * pow(1.0 - normalDotView, 5.0);
    } else {
        totalInternalReflection = true;
    }

    float r = rand(payload.rngState);
    vec3 newDir;

    if(totalInternalReflection || r < fresnel) {
        newDir = reflection;
    } else {
        newDir = T;
        payload.attenuation *= glassTint;
    }

    vec3 newOrigin = worldPos + sign(dot(newDir, worldNormal)) * worldNormal * 0.001;

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