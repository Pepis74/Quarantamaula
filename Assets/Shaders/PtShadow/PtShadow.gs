#version 460 core 

// Input is a triangle
layout (triangles) in;
// Output is 6 triangles (18 vertices), one for each face of the cubemap
layout (triangle_strip, max_vertices=18) out;

// Matrices used to transform vertices into the light space of each of the cubemap faces
uniform mat4 ptLightSpaceMats[6];

// Fragment's position in world space
out vec4 fragPos; 

void main()
{
    for(int face = 0; face < 6; face++)
    {
        // Set the cubemap face we're gonna render to
        gl_Layer = face; 

        // For each of the vertices in a triangle pass along their world space position and transform them into light space
        for(int i = 0; i < 3; ++i)
        {
            fragPos = gl_in[i].gl_Position;
            gl_Position = ptLightSpaceMats[face] * fragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  