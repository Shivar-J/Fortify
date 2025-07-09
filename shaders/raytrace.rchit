#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT RayPayload payload;
layout(location = 1) rayPayloadInEXT RayPayload reflectPayload;
layout(location = 2) rayPayloadInEXT RayPayload refractPayload;
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
layout(set = 0, binding = 9) uniform sampler2D modelTexture;

const float IOR = 1.5;
const vec3 glassTint = vec3(0.95, 0.98, 1.0);
const vec3 metalTints[4] = vec3[4](vec3(1.0, 0.71, 0.29), vec3(0.95, 0.64, 0.54), vec3(0.95), vec3(0.56, 0.57, 0.58));

void main() {
    if (payload.depth >= ubo.rayBounces) {
        payload.color = vec3(0);
        return;
    }

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

    vec2 uv0 = vertexBuffers[nonuniformEXT(instID)].vertices[i0].texCoord;
    vec2 uv1 = vertexBuffers[nonuniformEXT(instID)].vertices[i1].texCoord;
    vec2 uv2 = vertexBuffers[nonuniformEXT(instID)].vertices[i2].texCoord;

    vec3 bary = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 hitPoint = bary.x * p0 + bary.y * p1 + bary.z * p2;
    vec2 uv = uv0 * (1.0 - attribs.x - attribs.y) + uv1 * attribs.x + uv2 * attribs.y;

    mat3 normalMatrix = transpose(inverse(mat3(transform)));
    vec3 n0 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i0].normal);
    vec3 n1 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i1].normal);
    vec3 n2 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i2].normal);
    vec3 normal = normalize(bary.x * n0 + bary.y * n1 + bary.z * n2);

    vec3 viewDir = -gl_WorldRayDirectionEXT;

    if(isGlass) {
        float cosTheta = dot(normal, viewDir);
        float eta = cosTheta < 0.0 ? IOR : 1.0 / IOR;
        vec3 n = cosTheta < 0.0 ? -normal : normal;
        cosTheta = abs(cosTheta);

        vec3 R = reflect(-viewDir, n);
        vec3 T = refract(-viewDir, n, eta);
        bool TIR = length(T) == 0;

        float R0 = pow((1.0 - eta) / (1.0 + eta), 2.0);
        float fresnel = R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5.0);

        reflectPayload = payload;
        reflectPayload.color = vec3(0.0);
        reflectPayload.depth = payload.depth + 1;
        reflectPayload.rngState += 1;
    
        traceRayEXT(
            topLevelAS,
            gl_RayFlagsNoneEXT,
            0xFF,
            0, 0, 0,
            hitPoint + 1e-9 * R,
            1e-9,
            R,
            1e9,
            1
        );

        refractPayload = payload;
        refractPayload.color = vec3(0.0);
        refractPayload.depth = payload.depth + 1;
        refractPayload.rngState += 1;

        if(!TIR) {
            traceRayEXT(
                topLevelAS,
                gl_RayFlagsCullBackFacingTrianglesEXT,
                0xFF,
                0, 0, 0,
                hitPoint + 1e-9 * T,
                1e-9,
                T,
                1e9,
                2
            );
        }

        payload.attenuation *= vec3(1.0);
       
        vec3 surfaceColor = fresnel * reflectPayload.color + (1.0 - fresnel) * refractPayload.color;
        payload.color = surfaceColor * payload.attenuation;
    } else {
        vec3 albedo = texture(modelTexture, uv).rgb;

        vec3 reflected = reflect(viewDir, normal);
        reflectPayload = payload;
        reflectPayload.color = vec3(0.0);
        reflectPayload.depth = payload.depth + 1;
        reflectPayload.rngState += 3;

        traceRayEXT(
            topLevelAS,
            gl_RayFlagsNoneEXT,
            0xFF,
            0, 0, 0,
            hitPoint + 1e-9 * reflected,
            1e-9,
            reflected,
            1e9,
            1
        );

        payload.color = albedo;

       //payload.color = albedo * (reflectPayload.color * vec3(1.0, 0.0, 0.0)) * payload.attenuation;
    }

    payload.depth++;
}