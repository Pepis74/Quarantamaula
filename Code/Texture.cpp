#include "Texture.h"

//@Params: image's filepath. Creates and configurates texture object and attaches image to texture object 
Texture::Texture(const char* filePath, const char* inType) :  type(inType)
{
	//Obtain the image's raw data using stb_image
	int width, height, nChannels;
	unsigned char* imageData = stbi_load(filePath, &width, &height, &nChannels, 0);

	if (imageData) {
		//Generate and bind the texture object
		glGenTextures(1, &iD);
		glBindTexture(GL_TEXTURE_2D, iD);

		if (type == "diffuse") {
			//Create the actual texture in the newly created texture object using the sRGB color space to not gamma correct textures twice
			switch (nChannels) {
				case 1:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, imageData);
					break;
				case 3:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
					break;
				case 4:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
					break;
			}
		}

		else {
			//Create the actual texture in the newly created texture object in linear color space
			switch (nChannels) {
				case 1:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, imageData);
					break;
				case 3:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
					break;
				case 4:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
					break;
			}
		}

		//Create mipmap and set the texture wrap and scaling settings
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	else {
		std::cout << "Failed to load texture" << std::endl;
	}

	//Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(imageData);
}

unsigned int Texture::getID()
{
	return iD;
}

const std::string& Texture::getType()
{
	return type;
}
