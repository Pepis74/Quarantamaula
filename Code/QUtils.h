#ifndef Q_UTILS_H
#define Q_UTILS_H

#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <glad/glad.h>

class QUtils {
public:
	// Bind a texture to a texture unit and send that information to the shader. Shader program must be set in use beforehand
	static void bindTexture2D(unsigned int shaderID, const char* uniformName, unsigned int textureUnit, unsigned int texture) 
	{
		glUniform1i(glGetUniformLocation(shaderID, uniformName), textureUnit);
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, texture);
		//std::cout << uniformName << " texture bound at texture unit " << textureUnit << std::endl;
	}

	static void bindTexture3D(unsigned int shaderID, const char* uniformName, unsigned int textureUnit, unsigned int texture)
	{
		glUniform1i(glGetUniformLocation(shaderID, uniformName), textureUnit);
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_3D, texture);
		//std::cout << uniformName << " texture bound at texture unit " << textureUnit << std::endl;
	}

	// Bind a texture to a texture unit and send that information to the shader. Shader program must be set in use beforehand
	static void bindTextureCubemap(unsigned int shaderID, const char* uniformName, unsigned int textureUnit, unsigned int texture)
	{
		glUniform1i(glGetUniformLocation(shaderID, uniformName), textureUnit);
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		//std::cout << uniformName << " texture bound at texture unit " << textureUnit << std::endl;
	}
};

#endif
