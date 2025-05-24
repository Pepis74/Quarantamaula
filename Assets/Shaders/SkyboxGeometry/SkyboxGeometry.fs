#version 460 core

layout (location = 0) out vec4 gPositionRoughness;
layout (location = 1) out vec4 gNormalsSpecular;
layout (location = 2) out vec4 gTangents;
layout (location = 3) out vec4 gAlbedoAnisotropic;
layout (location = 4) out vec4 gMetallicClearcoatClearcoatroughness;

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

in vec3 texCoord;

uniform samplerCube skybox;

void main()
{   
    // Tone map HDR environment map
    vec3 hdrColor = texture(skybox, texCoord).rgb;
    vec3 mappedColor = vec3(1.0) - exp(-hdrColor * LPC_exposure);

    gAlbedoAnisotropic = vec4(mappedColor, 0.0);
}