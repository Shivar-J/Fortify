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

const float PI = 3.14159265359;

void main() {
    if (payload.depth >= ubo.rayBounces) {
        payload.color = vec3(0);
        return;
    }

    uint primID = gl_PrimitiveID;
    uint instID = gl_InstanceCustomIndexEXT;
    bool isPBR = false;
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

    mat3 normalMatrix = transpose(inverse(mat3(transform)));
    vec3 n0 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i0].normal);
    vec3 n1 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i1].normal);
    vec3 n2 = normalize(normalMatrix * vertexBuffers[nonuniformEXT(instID)].vertices[i2].normal);
    vec3 normal = normalize(bary.x * n0 + bary.y * n1 + bary.z * n2);

    vec3 viewDir = normalize(-gl_WorldRayDirectionEXT);

    vec3 albedo = (flagBits & ALBEDO_FLAG) != 0 ? texture(albedoTextures[nonuniformEXT(instID)], uv).rgb : vec3(1.0);
    //vec3 normalMap = (flagBits & NORMAL_FLAG) != 0 ? texture(normalTextures[nonuniformEXT(instID)], uv).rgb: normal;
    float roughness = (flagBits & ROUGHNESS_FLAG) != 0 ? texture(roughnessTextures[nonuniformEXT(instID)], uv).r : 0.0;
    float metalness = (flagBits & METALNESS_FLAG) != 0 ? texture(metalnessTextures[nonuniformEXT(instID)], uv).r : 0.0;
    float specular = (flagBits & SPECULAR_FLAG) != 0 ? texture(specularTextures[nonuniformEXT(instID)], uv).r : 0.0;
    float height = (flagBits & HEIGHT_FLAG) != 0 ? texture(heightTextures[nonuniformEXT(instID)], uv).r : 0.0;
    float ao = (flagBits & AMBIENT_OCCLUSION_FLAG) != 0 ? texture(ambientOcclusionTextures[nonuniformEXT(instID)], uv).r : 1.0;

    if(((flagBits & ALBEDO_FLAG) != 0) && (((flagBits & NORMAL_FLAG) != 0) || ((flagBits & ROUGHNESS_FLAG) != 0) || ((flagBits & METALNESS_FLAG) != 0) || ((flagBits & SPECULAR_FLAG) != 0) || ((flagBits & AMBIENT_OCCLUSION_FLAG) != 0))) {
        isPBR = true;
    }

    if ((flagBits & NORMAL_FLAG) != 0) {
        vec3 normalMap = texture(normalTextures[nonuniformEXT(instID)], uv).rgb;
        normalMap = normalize(normalMap * 2.0 - 1.0);

        normal = normalMap;
    }

    if (isPBR) {
        vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));
        vec3 lightColor = vec3(0.55);
        
        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metalness);

        vec3 V = viewDir;
        vec3 N = normal;
        vec3 L = lightDir;
        vec3 H = normalize(V + L);

        float NdotV = max(dot(N, V), 0.0001);
        float NdotL = max(dot(N, L), 0.0001);
        float NdotH = max(dot(N, H), 0.0001);
        float HdotV = max(dot(H, V), 0.0001);
    
        float a = roughness * roughness;
        float a2 = a * a;
        float denomNDF = NdotH * NdotH * (a2 - 1.0) + 1.0;
        float NDF = a2 / (PI * denomNDF * denomNDF);
    
        float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
        float G1 = NdotV / (NdotV * (1.0 - k) + k);
        float G2 = NdotL / (NdotL * (1.0 - k) + k);
        float G = G1 * G2;
    
        vec3 F = F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);
    
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * NdotV * NdotL;
        vec3 specular = numerator / max(denominator, 0.001);
    
        vec3 kD = (vec3(1.0) - F) * (1.0 - metalness);
        vec3 diffuse = kD * albedo / PI;
    
        vec3 radiance = lightColor * NdotL;
        vec3 Lo = (diffuse + specular) * radiance;
    
        vec3 ambient = vec3(0.15) * albedo * ao;
    
        payload.color = (ambient + Lo) * payload.attenuation;

    } else {
        payload.color = albedo * payload.attenuation;
    }

    payload.depth++;
}