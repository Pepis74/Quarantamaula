#version 460 core

// Position data
layout(location = 0) in vec3 inPosition;
// Normal vectors data (used for lighting)
layout(location = 1) in vec3 inNormal;
// Texture coordinates data
layout(location = 2) in vec2 inTexCoord;
// Tangent vectors data for normal mapping
layout(location = 3) in vec4 inTangent;

layout(std140, binding = 0) uniform VPMatrices //Size 128B
{
    // The matrix used to convert the vertices from world space to view space
    mat4 VPM_view; //B 0
    // The matrix used to convert the vertices from view space to clip space
    mat4 VPM_projection; //B 64
};

// Used to pass the texture coordinates along to the fragment shader
out vec2 texCoord;
// The position in world space of the vertex we're currenty rendering
out vec3 fragPos;
// The tangent vector alongside the bitangent handedness data
out vec4 tangent;
// Matrix used to transform normal vectors from a normal map to world space, and pass along the model's T, B,
out mat3 TBN;

// The matrix used to convert the vertices from local space to world space
uniform mat4 model;
// The matrix used to convert true normal vectors from local space to world space
uniform mat3 normalMat;  

void main()
{
    // Pass along the texture coordinates and tangents to the fragment shader
    texCoord = inTexCoord;
    tangent = inTangent;

    // Transform normal and tangent vectors to world space
    vec3 normal = normalize(normalMat * inNormal);
    vec3 tangentVec3 = normalize(normalMat * tangent.xyz);
    // Compute the bitangent as the cross product between the normal and the tangent, multiplied by MikkTSpace handedness for consistent right-handed tangent spaces
    vec3 bitangent = cross(normal, tangentVec3) * tangent.w;
    // Compute the TBN matrix
    TBN = mat3(tangentVec3, bitangent, normal);

    // Obtain the position of the current vertex in world space and pass it along to the fragment shader to use in lighting calculations 
    fragPos = vec3(model * vec4(inPosition, 1.0));
    // Compute the position of the fragment in clip space, which is the one OpenGL needs
    gl_Position = VPM_projection * VPM_view * vec4(fragPos, 1.0);
}