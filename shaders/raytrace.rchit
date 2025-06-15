#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : enable

layout(location = 0) rayPayloadInEXT vec3 payload;
hitAttributeEXT vec2 attribs;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoord;
};

layout(set = 0, binding = 4, std430) buffer Vertices {
    Vertex vertices[];
};

layout(set = 0, binding = 5, std430) buffer Indices {
    uint indices[];
};

void main() {
    uint baseIdx = 3 * gl_PrimitiveID;
    uint i0 = indices[baseIdx];
    uint i1 = indices[baseIdx + 1];
    uint i2 = indices[baseIdx + 2];
    
    Vertex v0 = vertices[i0];
    Vertex v1 = vertices[i1];
    Vertex v2 = vertices[i2];
    
    vec3 edge1 = v1.position - v0.position;
    vec3 edge2 = v2.position - v0.position;
    vec3 faceNormal = normalize(cross(edge1, edge2));
    
    vec3 normal;
    if (length(v0.normal) > 0.1 && length(v1.normal) > 0.1 && length(v2.normal) > 0.1) {
        normal = normalize(v0.normal * (1.0 - attribs.x - attribs.y) + 
                 v1.normal * attribs.x + 
                 v2.normal * attribs.y);
    } else {
        normal = faceNormal;
    }
    
    mat3 objectToWorld = mat3(gl_ObjectToWorldEXT);
    mat3 normalMatrix = transpose(inverse(objectToWorld));
    normal = normalize(normalMatrix * normal);
    
    vec3 viewDir = -gl_WorldRayDirectionEXT;
    if (dot(normal, viewDir) < 0.0) {
        normal = -normal;
    }
    
    vec3 hitPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
    
    vec3 lightPos = gl_WorldRayOriginEXT;
    vec3 toLight = lightPos - hitPos;
    float dist = length(toLight);
    vec3 lightDir = toLight / dist;
    
    float diff = max(dot(normal, -lightDir), 0.0);
    float intensity = 100.0;
    float attenuation = intensity / (dist * dist + 0.1);
    vec3 diffuse = vec3(1.0) * diff * attenuation;
    
    vec3 ambient = vec3(0.75);
    
    vec3 baseColor = vec3(0.7, 0.7, 0.7);
    
    payload = baseColor * (ambient + diffuse);
    
    // For normal visualization (uncomment to debug):
    // payload = normal * 0.5 + 0.5;
}