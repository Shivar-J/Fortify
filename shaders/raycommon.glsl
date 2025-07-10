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

struct TextureMapFlags {
    bool hasAlbedo;
    bool hasNormal;
    bool hasRoughness;
    bool hasMetalness;
    bool hasSpecular;
    bool hasHeight;
    bool hasAmbientOcclusion;
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

vec3 randomInUnitDisk(inout uint rngState) {
    float a = rand(rngState) * 2.0 * 3.1415926535;
    float r = sqrt(rand(rngState));
    return vec3(r * cos(a), r * sin(a), 0);
}