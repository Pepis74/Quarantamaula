#version 460 core

// Position data
layout(location = 0) in vec3 inPosition;

// The matrix used to convert the vertices from local space to world space
uniform mat4 model;

void main()
{
    // Transforming vertices from local to world space to be dealt with by the geometry shader
    gl_Position = model * vec4(inPosition, 1.0);
}  