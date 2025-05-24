#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "Model.h"

class ResourceManager {
public:
	//@Params : vertex shader filepath, fragment shader filepath, shader name for the map. Reads the shader source files, constructs a shader with that source code and then adds it to 
	//the shaders map
	static Shader* const loadShader(const char* vertexPath, const char* fragmentPath, const std::string& name);
	//@Params : vertex shader filepath, geometry shader filepath, fragment shader filepath, shader name for the map. Reads the shader source files, constructs a shader with that source code and then adds
	//it to the shaders map
	static Shader* const loadShader(const char* vertexPath, const char* geometryPath, const char* fragmentPath, const std::string& name);
	static Model* const loadModel(const char* filePath, const std::string& name);
	// Loads an hdr environment texture
	static unsigned int loadEnvTexture(const char* filePath, const std::string& name, bool flipVertically);
	// Loads an hdr environment cubemap. Filepath is the folder where all the cubemap faces must be in. They must be called nx, ny, ... and so on
	static unsigned int loadEnvCubemap(const char* filePath, const std::string& name, bool flipVertically);
	static Shader* const getShader(const std::string& name);
	//@Params : name of the model we want to retrieve. Returns a specific model from the loaded models
	static Model* const getModel(const std::string& name);
	static unsigned int getEnvTexture(const std::string& name);
	//Deletes all the data stored
	static void clear();

private:
	//Private constructor, no instancing
	ResourceManager() {}
	//Loaded shaders
	static std::map<std::string, Shader> shaders;
	//Loaded models
	static std::map<std::string, Model> models;
	// Loaded environment textures
	static std::map<std::string, unsigned int> envTextures;
};

#endif
