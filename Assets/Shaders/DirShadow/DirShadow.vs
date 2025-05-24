#version 460 core

// Position data
layout(location = 0) in vec3 inPosition;

layout(std140, binding = 3) uniform LightingMisc //Size 76B
{
    // The matrix used to convert fragment positions in world space to the directional light space
    mat4 LM_dirLightSpaceMat; //B 0
    // The position of the camera in world space 
    vec3 LM_viewPos; //B 64
};

// The matrix used to convert the vertices from local space to world space
uniform mat4 model;

void main()
{
    // Transform vertex to clip space
    gl_Position = LM_dirLightSpaceMat * model * vec4(inPosition, 1.0);
}