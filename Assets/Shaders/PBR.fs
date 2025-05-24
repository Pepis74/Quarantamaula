#version 460 core

layout(std140, binding = 0) uniform VPMatrices //Size 128B
{
    // The matrix used to convert the vertices from world space to view space
    mat4 VPM_view; //B 0
    // The matrix used to convert the vertices from view space to clip space
    mat4 VPM_projection; //B 64
};

layout(std140, binding = 1) uniform DirectionalLight //Size 28B
{
    // The light's direction
    vec3 DL_direction; //B 0
    // The strength of the light
    float DL_intensity; //B 12
    // The light's color
    vec3 DL_color; //B 16
};

layout(std140, binding = 2) uniform PointLights //Size 248B
{
    // The light's position
    vec3 PL_position[5]; //B 0
    // The light's color
    vec3 PL_color[5]; //B 80
    // The linear term in the attenuation function
    float PL_intensity[5]; //B 160
    // The number of point lights 
    uint PL_num; //B 240
    // The position of the far plane in the projection matrix used to render the depth cubemaps
    float PL_farPlane; //B 244
};

layout(std140, binding = 3) uniform LightingMisc //Size 76B
{
    // The matrix used to convert fragment positions in world space to the directional light space
    mat4 LM_dirLightSpaceMat; //B 0
    // The position of the camera in world space 
    vec3 LM_viewPos; //B 64
};

layout(std140, binding = 4) uniform FilterMapParams //Size 16B
{
    // Number of filters of the filterMap texture
    uint FMP_mapSize; //B 0
    // Number of samples per axis of a filter
    uint FMP_filterSize; //B 4
    // Radius of the filters stored in the filterMap texture
    float FMP_dirRadius; //B 8
    // Radius of the filters stored in the filterMap texture
    float FMP_ptRadius; //B 12
};

layout(std140, binding = 5) uniform LightingPassCommon //Size 16B
{
    // Index that determines what to show on screen. 0 - regular render, 1 - G Position, 2 - G Normals, 3 - G Albedo, 4 - G Specular,
    // 5 - Shader Mask
    uint LPC_showIndex; //B 0
    // Exposure parameter for HDR tone mapping
    float LPC_exposure; //B 4
    // Gamma constant for gamma correction
    float LPC_gamma; //B 8
    // Gamma constant for gamma correction
    uint LPC_roughnessMipLevels; //B 12
};

// Texture coordinates obtained from the vertex shader
in vec2 texCoord;

// The resulting color of the pixel currently being rendered
out vec4 fragColor;

uniform sampler2D gPositionRoughness;
//
uniform sampler2D gNormalsSpecular;
//
uniform sampler2D gTangents;
//
uniform sampler2D gAlbedoAnisotropic;
//
uniform sampler2D gMetallicClearcoatClearcoatroughness;
//
uniform sampler2D shaderMask;
// The irradiance map for diffuse ambient Lighting
uniform samplerCube irradianceMap;
// The irradiance map for diffuse ambient Lighting
uniform samplerCube specularMap;
// The irradiance map for diffuse ambient Lighting
uniform sampler2D brdfIntegrationMap;
// The depth map we sample to determine whether this fragment is in shadow in respect to the directional light
uniform sampler2D dirShadowMap;
// The depth cubemaps we sample to determine whether this fragment is in shadow in respect to each point light
uniform samplerCube ptShadowMaps[5];
// Texture that stores the filters used to test whether a fragment is in shadow
uniform sampler3D filterMap;

// PBR MODEL PARAMETERS
// The surface's color. Diffuse reflectance sigma
vec3 baseColor;
// 
float roughness;
// Controls the base reflectance F0 of the fresnel function
float specular;
// Higher metallic v
float metallic;
// Amount of anisotropy: a scalar between -1 and 1. Negative values will align the anisotropy with the bitangent direction instead of the tangent direction.
float anisotropic;
//
float clearcoat;
//
float clearcoatRoughness;

//GLOBAL VARIABLES
// The position in world space of the fragment we're currenty rendering
vec3 position;
// The position in the filter map of the filter mapped to this fragment
vec2 filterPos;
//
const float PI = 3.1415927;

const float AMBIENT_STRENGTH = 0.03;

const float SHADOW_BIAS = 0.02;

// Schlick's fresnel approximation
vec3 FSchlick(vec3 F0, vec3 F90, float cos) 
{
     return F0 + (F90 - F0) * pow(1.0 - cos, 5.0);
}
vec3 FSchlickRoughness(vec3 F0, float cos, float layerRoughness)
{
    return F0 + (max(vec3(1.0 - layerRoughness), F0) - F0) * pow(clamp(1.0 - cos, 0.0, 1.0), 5.0);
}   

// Lambert Diffuse BRDF
vec3 fdLambert() 
{
    return baseColor / PI;
}

// Burley's anisotropic GGX NDF
float DAnisoGGX(const vec3 H, const vec3 T, const vec3 B, float alphax, float alphay, float NdotH) 
{
    float TdotH = dot(T, H);
    float BdotH = dot(B, H);
    float a2 = alphax * alphay;
    vec3 v = vec3(alphay * TdotH, alphax * BdotH, a2 * NdotH);
    float v2 = dot(v, v);
    float w2 = a2 / v2;
    return a2 * w2 * w2 * (1.0 / PI);
}

// GGX NDF
float DGGX(float alpha, float NdotH) 
{
    float a2 = alpha * alpha;
    float f = (NdotH * a2 - NdotH) * NdotH + 1.0;
    return a2 / (PI * f * f);
}

// Heitz's anisotropic masking-shadowing function that matches GGX
float VAnisoGGX(const vec3 V, const vec3 L, const vec3 T, const vec3 B, float alphax, float alphay, float NdotV, float NdotL)
{
    //
    float TdotV = abs(dot(T, V)) + 1e-5;
    float BdotV = abs(dot(B, V)) + 1e-5;
    float TdotL = clamp(dot(T, L), 0.0, 1.0);
    float BdotL = clamp(dot(B, L), 0.0, 1.0);
    float lambdaV = NdotL * length(vec3(alphax * TdotV, alphay * BdotV, NdotV));
    float lambdaL = NdotV * length(vec3(alphax * TdotL, alphay * BdotL, NdotL));
    float v = 0.5 / (lambdaV + lambdaL);
    return v;
}

// FAST Smith's masking-shadowing function that matches GGX
float VGGX(float alpha, float NdotV, float NdotL) 
{
    float GGXV = NdotL * (NdotV * (1.0 - alpha) + alpha);
    float GGXL = NdotV * (NdotL * (1.0 - alpha) + alpha);
    return 0.5 / (GGXV + GGXL);
}

// For clearcoat
float VKelemen(float LdotH) {
    return 0.25 / (LdotH * LdotH);
}

vec3 computeAmbient(const vec3 N, const vec3 V, const vec3 F0, const vec3 F0c, float NdotV) 
{
    // Compute diffuse lighting coefficient
    vec3 F = FSchlickRoughness(F0, NdotV, roughness);
    vec3 kd = (1.0 - F) * (1.0 - metallic);
    // Compute diffuse component
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * baseColor;

    // Compute base specular component
    vec3 R = reflect(-V, N);
    vec3 specularMapColor = textureLod(specularMap, R,  roughness * LPC_roughnessMipLevels).rgb;   
    vec2 brdf = texture(brdfIntegrationMap, vec2(NdotV, roughness)).rg;
    vec3 specular = specularMapColor * (F * brdf.x + brdf.y);

    //Compute total base layer
    vec3 baseLayer = diffuse * kd + specular;

    // Compute clearcoat specular component
    vec3 Fc = FSchlickRoughness(F0c, NdotV, clearcoatRoughness) * clearcoat;
    specularMapColor = textureLod(specularMap, R,  clearcoatRoughness * LPC_roughnessMipLevels).rgb;   
    brdf = texture(brdfIntegrationMap, vec2(NdotV, clearcoatRoughness)).rg;
    vec3 clearcoatSpecular = specularMapColor * (Fc * brdf.x + brdf.y);

    //Compute total ambient component
    return (baseLayer * (1.0 - Fc) + clearcoatSpecular) * AMBIENT_STRENGTH;
}

vec3 BRDF(const vec3 N, const vec3 T, const vec3 B, const vec3 V, const vec3 L, const vec3 F0, const vec3 F0c, float NdotL, float NdotV) 
{
    // Remapping of roughness to obtain alpha
    float alpha = roughness * roughness + 1e-3;

    // Compute the directional roughness parameters
    float aspect = sqrt(1.0 - 0.9 * anisotropic);
    float alphax = alpha / aspect;
    float alphay = alpha * aspect;

    // Compute halfway vector. Direction of perfect reflection 
    vec3 H = normalize(V + L);
    // Compute relevant dot products
    // 
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float LdotH = clamp(dot(L, H), 0.0, 1.0);

    // Compute the diffuse BRDF of the base layer fd 
    vec3 fd = fdLambert();

    // Compute the specular BRDF of the base layer fs
    vec3 F = FSchlick(F0, vec3(1.0), LdotH);
    // Compute the single scattering model
    vec3 fss = vec3(DAnisoGGX(H, T, B, alphax, alphay, NdotH) * F * VAnisoGGX(V, L, T, B, alphax, alphay, NdotV, NdotL));
    // (WIP) Compute the multiple scattering model that we will later add to the single scattering model to prevent energy loss
    vec3 fms = vec3(0.0);
    // Add them together
    vec3 fs = fss + fms;

    // Compute the base layer's BRDF
    // For energy conversation, linearly interpolate between diffuse and specular contributions based on fresnel and metallicness
    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 fbase = kd * fd + fs;

    // Compute the clearcoat layer's BRDF fc
    // Remapping of clearcoatRoughness to obtain the alpha used for the clearcoat model calculations
    float alphac = clearcoatRoughness * clearcoatRoughness + 1e-3;
    // Compute the fresnel term of the clearcoat BRDF that will also be used later to account for energy loss in the base layer
    float Fc = FSchlick(F0c, vec3(1.0), LdotH).r * clearcoat;
    // Compute the rest and add it up 
    float fc = DGGX(alphac, NdotH) * Fc * VKelemen(LdotH);

    //Add up both layers while accounting for enegery preservation
    vec3 f = fbase * (1.0 - Fc) + fc;
    
    return f;
}

float computePtShadow(uint index, const vec3 lightDir, float currentDepth) 
{
    //In shadow = 0, not in shadow = 1
    float shadow = 1.0;

    //If the fragment is outside the range where shadows are rendered, it shouldn't be in shadow
    if(currentDepth <= PL_farPlane) 
    {
        // Shadow bias
        float SHADOW_BIAS = 0.01;
        // Invert lightDir vector to use as direction vector
        vec3 direction = -lightDir;
        // Where we'll store the depth we sample from the shadow map
        float sampledDepth = 0.0;

        // Compute the size of a texel in the shadow map
        vec2 texelSize = 1.0 / textureSize(ptShadowMaps[index], 0);
        // Which texel we are going to fetch from the filter map texture
        ivec3 filterMapTexelPos = ivec3(0, ivec2(filterPos));

        int i = 0;

        // Interate through the samples on the outer ring of the filter, testing them to see if they're in shadow
        for(i = 0; i < 4; i++) 
        {
            filterMapTexelPos.x = i;
            // Fetch the texel at the specified position
            // The rg components are the coordinates of one sample point, the ba components are the coordinates of another
            vec4 filterMapTexelContent = texelFetch(filterMap, filterMapTexelPos, 0) * FMP_ptRadius;

            // Sample the shadow map at the first sample point
            sampledDepth = texture(ptShadowMaps[index], direction + vec3(filterMapTexelContent.rg, 0) * vec3(texelSize, 1)).r;
            // Transform it from the [0,1] to its original range
            sampledDepth *= PL_farPlane;
            // If the current depth value is greater than the sampled one, this sample point is in shadow
            shadow += (currentDepth - SHADOW_BIAS) > sampledDepth ? 0.0 : 1.0;

            //Repeat the process for the other sample point
            sampledDepth = texture(ptShadowMaps[index], direction + vec3(filterMapTexelContent.ba, 0) * vec3(texelSize, 1)).r;
            sampledDepth *= PL_farPlane;
            shadow += (currentDepth - SHADOW_BIAS) > sampledDepth ? 0.0 : 1.0;
        }

        //If all the sample points in the outer ring don't have the same shadow value, keep testing the rest of the filter
        if(shadow != 8.0 && shadow != 0.0)
        {
            // We divide it by 2 because we test two samples each iteration
            uint maxIterations = (FMP_filterSize * FMP_filterSize) / 2;

            for(i = 4; i < maxIterations; i++) 
            {
                // We test each sample, same as the previous for loop
                filterMapTexelPos.x = i;
                vec4 filterMapTexelContent = texelFetch(filterMap, filterMapTexelPos, 0) * FMP_ptRadius;
                sampledDepth = texture(ptShadowMaps[index], direction + vec3(filterMapTexelContent.rg, 0) * vec3(texelSize, 1)).r;
                sampledDepth *= PL_farPlane;
                shadow += (currentDepth - SHADOW_BIAS) > sampledDepth ? 0.0 : 1.0;
                sampledDepth = texture(ptShadowMaps[index], direction + vec3(filterMapTexelContent.ba, 0) * vec3(texelSize, 1)).r;
                sampledDepth *= PL_farPlane;
                shadow += (currentDepth - SHADOW_BIAS) > sampledDepth ? 0.0 : 1.0;
            }
        }

        // Compute the average. We double i because in each iteration we tested two samples
        shadow /= i * 2;
    }

    return shadow;
}

float computeDirShadow() 
{
     // In shadow = 0, not in shadow = 1
    float shadow = 1.0;
    // Transform the fragment position to the directiona light space
    vec4 dirLightSpaceFragPos = LM_dirLightSpaceMat * vec4(position, 1.0);
    // Perform perspective division to transform coordinates to NDC range
    vec3 projectedFragPos = dirLightSpaceFragPos.xyz / dirLightSpaceFragPos.w;
    // Transform them to the range [0,1], which is the one the shadow map uses 
    projectedFragPos = projectedFragPos * 0.5 + 0.5;

    //If the fragment is outside the range where shadows are rendered, it shouldn't be in shadow
    if(projectedFragPos.z <= 1.0) 
    {
        // Shadow bias
        float SHADOW_BIAS = 0.01;
        // Get the depth value of this fragment
        float currentDepth = projectedFragPos.z;
        // Where we'll store the depth we sample from the shadow map
        float sampledDepth = 0.0;
        // Compute the size of a texel in the shadow map
        vec2 texelSize = 1.0 / textureSize(dirShadowMap, 0);

        // Which texel we are going to fetch from the filter map texture
        ivec3 filterMapTexelPos = ivec3(0, ivec2(filterPos));

        int i = 0;

        // Interate through the samples on the outer ring of the filter, testing them to see if they're in shadow
        for(i = 0; i < 4; i++) 
        {
            filterMapTexelPos.x = i;
            // Fetch the texel at the specified position
            // The rg components are the coordinates of one sample point, the ba components are the coordinates of another
            vec4 filterMapTexelContent = texelFetch(filterMap, filterMapTexelPos, 0) * FMP_dirRadius;

            // Sample the shadow map at the first sample point
            sampledDepth = texture(dirShadowMap, projectedFragPos.xy + filterMapTexelContent.rg * texelSize).r;
            // If the current depth value is greater than the sampled one, this sample point is in shadow
            shadow += (currentDepth - SHADOW_BIAS) > sampledDepth ? 0.0 : 1.0;

            //Repeat the process for the other sample point
            sampledDepth = texture(dirShadowMap, projectedFragPos.xy + filterMapTexelContent.ba * texelSize).r;
            shadow += (currentDepth - SHADOW_BIAS) > sampledDepth ? 0.0 : 1.0;
        }

        //If all the sample points in the outer ring don't have the same shadow value, keep testing the rest of the filter
        if(shadow != 8.0 && shadow != 0.0)
        {
            // We divide it by 2 because we test two samples each iteration
            uint maxIterations = (FMP_filterSize * FMP_filterSize) / 2;

            for(i = 4; i < maxIterations; i++) 
            {
                // We test each sample, same as the previous for loop
                filterMapTexelPos.x = i;
                vec4 filterMapTexelContent = texelFetch(filterMap, filterMapTexelPos, 0) * FMP_dirRadius;
                sampledDepth = texture(dirShadowMap, projectedFragPos.xy + filterMapTexelContent.rg * texelSize).r;
                shadow += (currentDepth - SHADOW_BIAS) > sampledDepth ? 0.0 : 1.0;
                sampledDepth = texture(dirShadowMap, projectedFragPos.xy + filterMapTexelContent.ba * texelSize).r;
                shadow += (currentDepth - SHADOW_BIAS) > sampledDepth ? 0.0 : 1.0;
            }
        }

        // Compute the average. We double i because in each iteration we tested two samples
        shadow /= i * 2;
    }

    return shadow;
}

vec3 computePtLight(const vec3 N, const vec3 T, const vec3 B, const vec3 V, const vec3 ambient, const vec3 F0, const vec3 F0c, float NdotV, uint index) 
{
    // Compute the light's direction and use it to compute the distance between the light and the fragment
    vec3 magnitudeL = PL_position[index] - position;
    float distance = length(magnitudeL);
    vec3 L = normalize(magnitudeL);
    // Compute the dot product between normal and light vectors
    float NdotL = clamp(dot(N, L), 0.0, 1.0);
    // Compute the attenuation
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance) + 1e-5);  
    // Compute the shadow factor
    float shadow = computePtShadow(index, magnitudeL, distance);
    //Compute incident radiance (ambient component not accounted for yet)
    vec3 Li = PL_color[index] * attenuation * shadow;
    // Compute outgoing radiance via the rendering equation (ambient component not accounted for yet)
    vec3 Lo = BRDF(N, T, B, V, L, F0, F0c, NdotL, NdotV) * Li * NdotL;
    // Add ambient component, multiply with intesity and return the result
    return (Lo + ambient) * PL_intensity[index];
}

vec3 computeDirLight(const vec3 N, const vec3 T, const vec3 B, const vec3 V, const vec3 ambient, const vec3 F0, const vec3 F0c, float NdotV) 
{
    // Normalize the light direction
    vec3 L = normalize(-DL_direction);
    // Compute the dot product between normal and light vectors
    float NdotL = clamp(dot(N, L), 0.0, 1.0);
    // Compute the shadow factor
    float shadow = computeDirShadow();
    //Compute incident radiance (ambient component not accounted for yet)
    vec3 Li = DL_color * shadow;
    // Compute outgoing radiance via the rendering equation (ambient component not accounted for yet)
    vec3 Lo = BRDF(N, T, B, V, L, F0, F0c, NdotL, NdotV) * Li * NdotL;
    // Add ambient component, multiply with intesity and return the result
    return (Lo + ambient) * DL_intensity;
}


void main()
{
    fragColor = vec4(0.0);

    // Only proceed if this fragment of the screen quad corresponds to an object being rendered with this shader
    if(texture(shaderMask, texCoord).b > 0.0) 
    {
        // Fetch the necessary data from the G buffer
        vec4 positionRoughness = texture(gPositionRoughness, texCoord);
        position = positionRoughness.rgb;
        roughness = positionRoughness.a;
        
        vec4 normalsSpecular = texture(gNormalsSpecular, texCoord);
        vec3 N = normalsSpecular.rgb;
        specular = normalsSpecular.a;

        vec4 tangents = texture(gTangents, texCoord);
        vec3 T = tangents.rgb;

        vec4 albedoAnisotropic = texture(gAlbedoAnisotropic, texCoord);
        baseColor = albedoAnisotropic.rgb;
        anisotropic = albedoAnisotropic.a;

        vec4 metallicClearcoatClearcoatroughness = texture(gMetallicClearcoatClearcoatroughness, texCoord);
        metallic = metallicClearcoatClearcoatroughness.r;
        clearcoat = metallicClearcoatClearcoatroughness.g;
        clearcoatRoughness = metallicClearcoatClearcoatroughness.b;

        // Compute view direction
        vec3 V = normalize(LM_viewPos - position);
        // Compute bitangent employing MikkTSpace handedness
        vec3 B = normalize(cross(N, T)) * tangents.a;

        // Map the current fragment to a filter in the filter map
        filterPos = mod(gl_FragCoord.xy, vec2(FMP_mapSize));

        // Compute the necessary values to compute the ambient component
        float NdotV = abs(dot(N, V)) + 1e-5;
        vec3 F0 = mix(vec3(0.16 * specular * specular), baseColor, metallic);
        vec3 F0c = vec3(0.04);
        vec3 ambient = computeAmbient(N, V, F0, F0c, NdotV);

        vec3 tempFragColor;

        // Add to the output the result of computing the scene's directional light if there's one
        if(DL_intensity != 0.0) tempFragColor = computeDirLight(N, T, B, V, ambient, F0, F0c, NdotV);

        uint i;

        // Loop through each point light in the scene and add its contribution to the output
        for(i = 0; i < PL_num; i++) tempFragColor += computePtLight(N, T, B, V, ambient, F0, F0c, NdotV, i);

        // Perform tone mapping
        tempFragColor = vec3(1.0) - exp(-tempFragColor * LPC_exposure);

        switch(LPC_showIndex) 
        {
        case 0:
            fragColor = vec4(tempFragColor, 1.0);
            //fragColor = vec4(ambient, 1.0);
            break;

        case 1:
            fragColor = vec4(position, 1.0);
            break;

        case 2:
            fragColor = vec4(N, 1.0);
            break;

        case 3:
            fragColor = vec4(T, 1.0);
            break;

        case 4:
            fragColor = vec4(baseColor, 1.0);
            break;

        case 5:
            fragColor = vec4(vec3(roughness), 1.0);
            break;

        case 6:
            fragColor = vec4(vec3(specular), 1.0);
            break;

        case 7:
            fragColor = vec4(vec3(metallic), 1.0);
            break;

        case 8:
            if (anisotropic > 0.0)
                fragColor = vec4(anisotropic, 0.0, 0.0, 1.0);
            else
                fragColor = vec4(0.0, 0.0, -anisotropic, 1.0);
            break;

        case 9:
            fragColor = vec4(vec3(clearcoat), 1.0);
            break;

        case 10:
            fragColor = vec4(vec3(clearcoatRoughness), 1.0);
            break;

        case 11:
            fragColor = vec4(0.0, 0.0, 1.0, 1.0);
            break;
        }

        // Gamma correction
        fragColor.rgb = pow(fragColor.rgb, vec3(1.0/LPC_gamma));
    }
}
