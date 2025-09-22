#version 460 core

layout(std140, binding = 0) uniform VPMatrices //Size 128B
{
    // The matrix used to convert the vertices from world space to view space
    mat4 VPM_view; //B 0
    // The matrix used to convert the vertices from view space to clip space
    mat4 VPM_projection; //B 64
};

// Texture coordinates obtained from the vertex shader
in vec2 texCoord;

// The resulting color of the pixel currently being rendered
out vec4 fragColor;

uniform sampler2D gPositionRoughness;
uniform sampler2D gNormalsSpecular;
uniform sampler2D gTangents;
uniform sampler3D kernelsTex;
uniform uint kernelSamples;
uniform uint kernelTexSize;
uniform float kernelRadius;
uniform float bias;

void main()
{
     // Map the current fragment to a kernel in the kernel texture
     vec2 kernelPos = mod(gl_FragCoord.xy, vec2(kernelTexSize));

     // Fetch G-Buffer data for the current fragment
     vec3 fragPos_World = texture(gPositionRoughness, texCoord).rgb;
     vec3 N_World = texture(gNormalsSpecular, texCoord).rgb;
     vec4 tangents_World = texture(gTangents, texCoord);
     vec3 T_World = tangents_World.rgb;
    
     // Convert all necessary data to View Space ONCE 
     vec3 fragPos = (VPM_view * vec4(fragPos_World, 1.0)).xyz;
    
     // For normals and tangents, we use the normal matrix (or the upper 3x3 of the view matrix) We renormalize to account for potential non-uniform scaling.
     mat3 view3x3 = mat3(VPM_view);
     vec3 N = normalize(view3x3 * N_World);
     vec3 T= normalize(view3x3 * T_World);
    
     // Recreate the TBN matrix directly in view space
     vec3 B = normalize(cross(N, T)) * tangents_World.a; // Preserve handedness
     mat3 TBN = mat3(T, B, N);

     // Iterate over the kernel samples and compute the occlusion factor
     float occlusion = 0.0;

     for(uint i = 0; i < kernelSamples; i++) { 
         // Obtain the kernel sample view position
         vec3 samplePos = TBN * texelFetch(kernelsTex, ivec3(i, ivec2(kernelPos)), 0).rgb;
         samplePos = fragPos + samplePos * kernelRadius;

         // Transform it to screen space coordinates to sample the depth at that point 
         vec4 offset = vec4(samplePos, 1.0);
         offset = VPM_projection * offset;    // from view to clip-space
         offset.xyz /= offset.w;               // perspective divide
         offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  

         // Obtain the depth at that sample point
         vec4 sampledCoordinates = vec4(texture(gPositionRoughness, offset.xy).rgb, 1.0); 
         float sampledDepth = (VPM_view * sampledCoordinates).z;

         // Make it so surfaces out of the kernel radius don't affect the occlusion factor
         float rangeCheck = smoothstep(0.0, 1.0, kernelRadius / abs(fragPos.z - sampledDepth));
         // Check if the sampled depth is greater than the z coordinate at the sample point
         occlusion += (sampledDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;   
     }

     // Normalize the occlusion factor and invert it to use it to scale ambient lighting
     float ambientScale = 1.0 - (occlusion / kernelSamples);
     fragColor = vec4(ambientScale);  
}