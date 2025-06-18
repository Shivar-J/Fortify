#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable

struct RayPayload {
    vec3 color;
    vec3 attenuation;
    uint depth;
};
layout(location = 0) rayPayloadInEXT RayPayload payload;
hitAttributeEXT vec2 attribs;

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

layout(set = 0, binding = 5, scalar) buffer Indices {
    uint indices[];
};

void main() {
    uint primID = gl_PrimitiveID;

    uint i0 = indices[primID * 3 + 0];
    uint i1 = indices[primID * 3 + 1];
    uint i2 = indices[primID * 3 + 2];

    vec3 p0 = vertices[i0].position;
    vec3 p1 = vertices[i1].position;
    vec3 p2 = vertices[i2].position;    

    vec3 n0 = vertices[i0].normal;
    vec3 n1 = vertices[i1].normal;
    vec3 n2 = vertices[i2].normal;

    float u = attribs.x;
    float v = attribs.y;
    float w = 1.0 - u - v;

    vec3 normal = normalize(w * n0 + u * n1 + v * n2);
    vec3 worldPos = w * p0 + u * p1 + v * p2;

    vec3 cameraPos = vec3(inverse(ubo.view)[3]);

    vec3 lightDir = normalize(cameraPos - worldPos);
    float diffuse = max(dot(normal, lightDir), 0.0);

    payload.color = vec3(diffuse);
}
