#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> 
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {

public:
	//Default constructor
	Shader() {}
	//@Params : vertex shader source code, fragment shader source code. Creates and links the shader program using the given shader source code
	Shader(const char* vertexCode, const char* fragmentCode);
	//@Params : vertex shader source code, geometry shader source code, fragment shader source code. Creates and links the shader program using the given shader source code
	Shader(const char* vertexCode, const char* geometryCode, const char* fragmentCode);
	//Returns the Shader's ID
	unsigned int getID();

private:
	
	//ID of the shader program object
	unsigned int iD = 0;
};

#endif
