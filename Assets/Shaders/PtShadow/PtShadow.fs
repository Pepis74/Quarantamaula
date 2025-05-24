#version 460 core

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

// Fragment's position in world space
in vec4 fragPos;

// Which point light we're rendering the shadow map for
uniform uint lightIndex;

void main()
{
    // Compute the distance between the point light and the current fragment
    float depthValue = length(fragPos.xyz - PL_position[lightIndex]);
    
    // Map the distance to the [0, 1] range used in the depth buffer and store it there
    depthValue = depthValue / PL_farPlane;
    gl_FragDepth = depthValue;
}  