#version 460 core

layout(std140, binding = 5) uniform LightingPassCommon //Size 16B
{
    // Index that determines what to show on screen. 0 - regular render, 1 - G Position, 2 - G Normals, 3 - G Albedo, 4 - G Specular,
    // 5 - Shader Mask
    uint LPC_showIndex; //B 0
    // Exposure parameter for HDR tone mapping
    float LPC_exposure; //B 4
    // Gamma constant for gamma correction
    float LPC_gamma; //B 8
    // Gamma constant for gamma correction
    uint LPC_roughnessMipLevels; //B 12
};

// Texture coordinates obtained from the vertex shader
in vec2 texCoord;

// The resulting color of the pixel currently being rendered
out vec4 fragColor;

uniform sampler2D gAlbedoAnisotropic;
uniform sampler2D shaderMask;

void main()
{
	fragColor = vec4(0.0);

	// Only proceed if this fragment of the screen quad corresponds to an object being rendered with this shader
	if(texture(shaderMask, texCoord).g > 0.0) 
	{
		vec3 color = texture(gAlbedoAnisotropic, texCoord).rgb;
	
		// Perform tone mapping
		color = vec3(1.0) - exp(-color * LPC_exposure);

		switch(LPC_showIndex) 
		{
		case 0:
			fragColor = vec4(color, 1.0);
			break;

		case 1:
			fragColor = vec4(0.0,0.0,0.0,1.0);
			break;

		case 2:
			fragColor = vec4(0.0,0.0,0.0,1.0);
			break;

		case 3:
			fragColor = vec4(0.0,0.0,0.0,1.0);
			break;

		case 4:
			fragColor = vec4(color, 1.0);
			break;

		case 5:
			fragColor = vec4(0.0,0.0,0.0,1.0);
			break;

		case 6:
			fragColor = vec4(0.0,0.0,0.0,1.0);
			break;

		case 7:
			fragColor = vec4(0.0,0.0,0.0,1.0);
			break;

		case 8:
			fragColor = vec4(0.0,0.0,0.0,1.0);
			break;

		case 9:
			fragColor = vec4(0.0,0.0,0.0,1.0);
			break;

		case 10:
			fragColor = vec4(0.0,0.0,0.0,1.0);
			break;

		case 11:
			fragColor = vec4(0.0, 1.0, 0.0, 1.0);
			break;
		}

		// Gamma correction
		fragColor.rgb = pow(fragColor.rgb, vec3(1.0/LPC_gamma));
	}
}
