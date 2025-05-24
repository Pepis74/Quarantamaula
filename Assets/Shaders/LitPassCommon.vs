#version 460 core

// Position data
layout(location = 0) in vec2 inPosition;
// Texture coordinates data
layout(location = 1) in vec2 inTexCoord;

// Used to pass the texture coordinates along to the fragment shader
out vec2 texCoord;

void main()
{
    // Pass texture coords along the pipeline
    texCoord = inTexCoord;

    // Position already in NDC
    gl_Position = vec4(inPosition, 0.0, 1.0);
}