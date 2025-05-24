#version 460 core

layout (location = 0) out vec4 gPositionRoughness;
layout (location = 1) out vec4 gNormalsSpecular;
layout (location = 2) out vec4 gTangents;
layout (location = 3) out vec4 gAlbedoAnisotropic;
layout (location = 4) out vec4 gMetallicClearcoatClearcoatroughness;

// Texture coordinates obtained from the vertex shader
in vec2 texCoord;
// The position in world space of the fragment we're currenty rendering
in vec3 fragPos;
// The tangent vector alongside the bitangent handedness data
in vec4 tangent;
// Matrix used to transform normal vectors from a normal map to world space, and to obtain the T, B and N vectors
in mat3 TBN;

// TEXTURES
// The mesh's normal map
uniform sampler2D normalMap;
// The mesh's diffuse texture
uniform sampler2D diffuseMap;
// The mesh's roughness map
uniform sampler2D roughnessMap;
// The mesh's specular map
uniform sampler2D specularMap;
// The mesh's metallic map
uniform sampler2D metallicMap;
// The mesh's anisotropic map
uniform sampler2D anisotropicMap;
// The mesh's clearcoat map
uniform sampler2D clearcoatMap;
// The mesh's clearcoat roughness map
uniform sampler2D clearcoatRoughnessMap;

// MAP BOOLEANS
// 1 => this mesh has a normal map, 0 otherwise
uniform uint hasNormalMap;
// 1 => this mesh has a diffuse map, 0 otherwise
uniform uint hasDiffuseMap;
// 1 => this mesh has a roughness map, 0 otherwise
uniform uint hasRoughnessMap;
// 1 => this mesh has a specular map, 0 otherwise
uniform uint hasSpecularMap;
// 1 => this mesh has a metallic map, 0 otherwise
uniform uint hasMetallicMap;
// 1 => this mesh has an anisotropic map, 0 otherwise
uniform uint hasAnisotropicMap;
// 1 => this mesh has a clearcoat map, 0 otherwise
uniform uint hasClearcoatMap;
// 1 => this mesh has a clearcoat roughness map, 0 otherwise
uniform uint hasClearcoatRoughnessMap;

// Lighting pass shader parameters that aren't passed through a texture
//
uniform vec3 baseColor;
//
uniform float roughness;
//
uniform float specular;
// Higher metallic v
uniform float metallic;
// Amount of anisotropy: a scalar between -1 and 1. Negative values will align the anisotropy with the bitangent direction instead of the tangent direction
uniform float anisotropic;
//
uniform float clearcoat;
//
uniform float clearcoatRoughness;

void main()
{   
    vec3 endNormal;

    // Sample the normal map 
    vec3 sampledNormal = texture(normalMap, texCoord).rgb;

    // Take the geometry normal if this mesh has no normal map
    if(hasNormalMap == 0) 
    {
       endNormal = normalize(TBN[2]);
    }

    // Take the normal from the normal map
    else
    {
        // Compute the appropriate transformations
        sampledNormal = sampledNormal * 2.0 - 1.0;   
        endNormal = normalize(TBN * sampledNormal);
    }
    
    // Diffuse (baseColor)
    vec3 endDiffuse = baseColor;
    if (hasDiffuseMap > 0) endDiffuse = texture(diffuseMap, texCoord).rgb;

    // Roughness
    float endRoughness = roughness;
    if (hasRoughnessMap > 0) endRoughness = texture(roughnessMap, texCoord).r;

    // Specular
    float endSpecular = specular;
    if (hasSpecularMap > 0) endSpecular = texture(specularMap, texCoord).r;

    // Metallic
    float endMetallic = metallic;
    if (hasMetallicMap > 0) endMetallic = texture(metallicMap, texCoord).r;

    // Anisotropic
    float endAnisotropic = anisotropic;
    if (hasAnisotropicMap > 0) endAnisotropic = texture(anisotropicMap, texCoord).r;

    // Clearcoat
    float endClearcoat = clearcoat;
    if (hasClearcoatMap > 0) endClearcoat = texture(clearcoatMap, texCoord).r;

    // Clearcoat Roughness
    float endClearcoatRoughness = clearcoatRoughness;
    if (hasClearcoatRoughnessMap > 0) endClearcoatRoughness = texture(clearcoatRoughnessMap, texCoord).r;

    // Store the fragment position vector in the rgb and the roughness in the a of the first G buffer texture
    gPositionRoughness = vec4(fragPos, endRoughness);
    // Store the per-fragment world space normalized normals in the rgb and the specular intensity in the a of the second G buffer
    gNormalsSpecular = vec4(endNormal, endSpecular);
    // Store the tangent in the rgba of the third G buffer
    gTangents = tangent;
    // Store the diffuse in the rgb and anisotropic in the a of the fourth G buffer
    gAlbedoAnisotropic = vec4(endDiffuse, endAnisotropic);
    // Store the metallic in the r, the clearcoat in the g and clearcoat roughness in the b of the fifth G buffer
    gMetallicClearcoatClearcoatroughness = vec4(endMetallic, endClearcoat, endClearcoatRoughness, 0.0);
}  