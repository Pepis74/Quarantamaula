#version 460 core
out vec4 fragColor;
in vec3 position;

uniform samplerCube envMap;
uniform float roughness;

const float RADIANCE_THRESHOLD = 10.0;
const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 1024u;

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
} 

float DGGX(float alpha, float NdotH) 
{
    float a2 = alpha * alpha;
    float f = (NdotH * a2 - NdotH) * NdotH + 1.0;
    return a2 / (PI * f * f);
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}  
  
void main()
{		
    vec3 N = normalize(position);
    vec3 V = N;

    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);

    // Build a non-degenerate tangent space
    vec3 helper = (abs(N.y) < 0.999) ? vec3(0.0,1.0,0.0)
                                     : vec3(1.0,0.0,0.0);
    vec3 tangentX = normalize(cross(helper, N));
    vec3 tangentY = normalize(cross(N, tangentX));

    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        // 1) sample H / L
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0) {
            float NdotH = max(dot(N, H),  0.0);
            float HdotV = max(dot(H, V),  0.0);

            // 2) PDF
            float alpha = roughness * roughness;
            float D   = DGGX(alpha, NdotH);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

            // 3) solid angles
            uint resolution = textureSize(envMap, 0).x;
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            // 4) mip level
            float mipLevel = (roughness == 0.0)
                ? 0.0
                : 0.5 * log2(saSample / saTexel);

            // 5) fetch and clamp to avoid fireflies
            vec3 radiance = min(textureLod(envMap, L, mipLevel).rgb, RADIANCE_THRESHOLD);

            // 6) accumulate
            prefilteredColor += radiance * NdotL;
            totalWeight      += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;
    fragColor = vec4(prefilteredColor, 1.0);
}  
  