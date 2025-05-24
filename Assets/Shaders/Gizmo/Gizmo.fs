#version 460 core

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

// Fragment color
out vec4 fragColor;

// Gizmo's color
uniform vec3 color;

void main()
{
    // Just return the color of the gizmo 
    fragColor = vec4(color, 1.0);
    
    // Gamma correction
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/LPC_gamma));
} 