#version 460 core

// Position data
layout(location = 0) in vec3 inPosition;
// Normal vectors data (used for lighting)
layout(location = 1) in vec3 inNormal;
// Texture coordinates data
layout(location = 2) in vec2 inTexCoord;

layout(std140, binding = 0) uniform VPMatrices //Size 128B
{
    // The matrix used to convert the vertices from world space to view space
    mat4 VPM_view; //B 0
    // The matrix used to convert the vertices from view space to clip space
    mat4 VPM_projection; //B 64
};

// The matrix used to convert the vertices from local space to world space
uniform mat4 model;

void main()
{
    // Transform vertex to clip space
    gl_Position = VPM_projection * VPM_view * model * vec4(inPosition, 1.0);
}