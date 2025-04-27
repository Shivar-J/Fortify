#version 450

layout(set = 0, binding = 1) uniform sampler2D diffuseMap;
layout(set = 0, binding = 2) uniform sampler2D normalMap;
layout(set = 0, binding = 3) uniform sampler2D roughnessMap;
layout(set = 0, binding = 4) uniform sampler2D metalnessMap;
layout(set = 0, binding = 5) uniform sampler2D aoMap;
layout(set = 0, binding = 6) uniform sampler2D specularMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 diffuse = texture(diffuseMap, fragTexCoord).rgb;
    //vec3 ao = texture(aoMap, fragTexCoord).rgb;
    //vec3 normal = texture(normalMap, fragTexCoord).rgb;
    //vec3 roughness = texture(roughnessMap, fragTexCoord).rgb;
    //vec3 specular = texture(specularMap, fragTexCoord).rgb;

    ///vec3 result = (diffuse + ao + specular) / 3.0;

    outColor = vec4(diffuse, 1.0);
}
