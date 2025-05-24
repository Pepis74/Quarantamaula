#version 460 core

// Fragment color
layout (location = 0) out vec4 mask;

// One-hot vector that determines the channel the shader mask will be stored in
uniform vec4 channel;

void main()
{
    // Just return on the corresponding channel
    mask = channel;
} 