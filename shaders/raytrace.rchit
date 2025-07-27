#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT RayPayload payload;
layout(location = 1) rayPayloadEXT RayPayload reflectPayload;
layout(location = 2) rayPayloadEXT RayPayload refractPayload;
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
layout(set = 0, binding = 9) uniform sampler2D albedoTextures[];
layout(set = 0, binding = 10) uniform sampler2D normalTextures[];
layout(set = 0, binding = 11) uniform sampler2D roughnessTextures[];
layout(set = 0, binding = 12) uniform sampler2D metalnessTextures[];
layout(set = 0, binding = 13) uniform sampler2D specularTextures[];
layout(set = 0, binding = 14) uniform sampler2D heightTextures[];
layout(set = 0, binding = 15) uniform sampler2D ambientOcclusionTextures[];
layout(set = 0, binding = 16) buffer Textures { uint flags[]; } textureFlags;

const uint ALBEDO_FLAG = 1u << 0;
const uint NORMAL_FLAG = 1u << 1;
const uint ROUGHNESS_FLAG = 1u << 2;
const uint METALNESS_FLAG = 1u << 3;
const uint SPECULAR_FLAG = 1u << 4;
const uint HEIGHT_FLAG = 1u << 5;
const uint AMBIENT_OCCLUSION_FLAG = 1u << 6;

const float IOR = 1.5;
const vec3 glassTint = vec3(0.95, 0.98, 1.0);
const vec3 metalTints[4] = vec3[4](vec3(1.0, 0.71, 0.29), vec3(0.95, 0.64, 0.54), vec3(0.95), vec3(0.56, 0.57, 0.58));
const float PI = 3.14159265359;

mat3 computeTBN(vec3 pos0, vec3 pos1, vec3 pos2, vec2 uv0, vec2 uv1, vec2 uv2, vec3 normal) {
    vec3 edge1 = pos1 - pos0;
    vec3 edge2 = pos2 - pos0;
    vec2 deltaUV1 = uv1 - uv0;
    vec2 deltaUV2 = uv2 - uv0;

    float denom = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
    float f = abs(denom) > 1e-6 ? 1.0 / denom : 1.0;

    vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
    vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

    tangent = normalize(tangent - dot(tangent, normal) * normal);
    bitangent = normalize(bitangent - dot(bitangent, normal) * normal - dot(bitangent, tangent) * tangent);

    return mat3(tangent, bitangent, normal);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float distributionGGX(vec3 N, vec3 H, float a) {
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float geometrySchlickGGX(float NdotV, float k) {
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float k) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = geometrySchlickGGX(NdotV, k);
    float ggx2 = geometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

void main() {
    if (payload.depth >= ubo.rayBounces) {
        payload.color = vec3(0);
        return;
    }

    uint primID = gl_PrimitiveID;
    uint instID = gl_InstanceCustomIndexEXT;
    bool isPBR = (instID % 2) == 0;

    uint flagBits = textureFlags.flags[nonuniformEXT(instID)];

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
    
    vec3 edge1 = p1 - p0;
    vec3 edge2 = p2 - p0;
    vec3 geometricNormal = normalize(cross(edge1, edge2));
    if (gl_HitKindEXT == gl_HitKindBackFacingTriangleEXT) {
        geometricNormal = -geometricNormal;
    }

    mat3 normalMatrix = transpose(inverse(mat3(transform)));
    vec3 n0 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i0].normal);
    vec3 n1 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i1].normal);
    vec3 n2 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i2].normal);
    vec3 normal = normalize(bary.x * n0 + bary.y * n1 + bary.z * n2);

    mat3 tbn = computeTBN(p0, p1, p2, uv0, uv1, uv2, normal);
    
    vec3 albedo = (flagBits & ALBEDO_FLAG) != 0 ? texture(albedoTextures[nonuniformEXT(instID)], uv).rgb : vec3(1.0);
    vec3 normalMap = (flagBits & NORMAL_FLAG) != 0 ? texture(normalTextures[nonuniformEXT(instID)], uv).rgb: normal;
    float roughness = (flagBits & ROUGHNESS_FLAG) != 0 ? texture(roughnessTextures[nonuniformEXT(instID)], uv).r : 0.0;
    float metalness = (flagBits & METALNESS_FLAG) != 0 ? texture(metalnessTextures[nonuniformEXT(instID)], uv).r : 0.0;
    float specular = (flagBits & SPECULAR_FLAG) != 0 ? texture(specularTextures[nonuniformEXT(instID)], uv).r : 0.0;
    float height = (flagBits & HEIGHT_FLAG) != 0 ? texture(heightTextures[nonuniformEXT(instID)], uv).r : 0.0;
    float ao = (flagBits & AMBIENT_OCCLUSION_FLAG) != 0 ? texture(ambientOcclusionTextures[nonuniformEXT(instID)], uv).r : 1.0;

    if((flagBits & NORMAL_FLAG) != 0) {
        vec3 tangentNormal = normalize(normalMap * 2.0 - 1.0);

        vec3 worldNormal = tbn * tangentNormal;
        
        if (dot(geometricNormal, worldNormal) < 0) {
            worldNormal = -worldNormal;
        }
        
        normal = normalize(worldNormal);
    }

    vec3 viewDir = normalize(-gl_WorldRayDirectionEXT);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metalness);

    if(isPBR) {
        vec3 reflectionDir = reflect(-viewDir, normal);
        vec3 incomingRadiance = vec3(0.0);

        if(metalness > 0.1 || specular > 0.1) {
            reflectPayload = payload;
            reflectPayload.depth = payload.depth + 1;
            
            float offsetSign = dot(geometricNormal, normal) > 0 ? 1.0 : -1.0;
            vec3 offsetOrigin = hitPoint + offsetSign * 1e-3 * geometricNormal;

            traceRayEXT(
                topLevelAS,
                gl_RayFlagsOpaqueEXT,
                0xFF,
                0, 0, 0,
                offsetOrigin,
                1e-4,
                reflectionDir,
                1e9,
                1
            );

            incomingRadiance = reflectPayload.color;
        }
        vec3 L = normalize(reflectionDir);
        vec3 H = normalize(viewDir + L);
        float NdotV = max(dot(normal, viewDir), 0.0);
        float NdotL = max(dot(normal, L), 0.0);
        float NdotH = max(dot(normal, H), 0.0);
        float HdotV = max(dot(H, viewDir), 0.0);

        float D = distributionGGX(normal, H, roughness);
        float G = geometrySmith(normal, viewDir, L, roughness);
        vec3 F = fresnelSchlick(HdotV, F0);

        vec3 specularTerm = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001);
        
        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metalness);
        vec3 diffuse = kD * albedo / PI;
        vec3 color = diffuse;
        if(metalness > 0.1) {
            color += specularTerm * incomingRadiance * NdotL;
        }
        color += vec3(0.03) * albedo * ao;
        color = color / (color + vec3(1.0));

        payload.color = color;
    } else {
        payload.color = albedo * payload.attenuation;
    }

    payload.depth++;
}