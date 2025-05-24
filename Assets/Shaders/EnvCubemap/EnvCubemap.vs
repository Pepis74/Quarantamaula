#version 460 core
layout (location = 0) in vec3 inPosition;

out vec3 position;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    position = inPosition;  
    gl_Position =  projection * view * vec4(position, 1.0);
}

