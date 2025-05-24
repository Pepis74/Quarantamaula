#ifndef ENV_TEXTURE_PREPROCESSOR_H
#define ENV_TEXTURE_PREPROCESSOR_H

#define IRRADIANCE_MAP_RES 64
#define SPECULAR_MAP_RES 128
#define ROUGHNESS_MIP_LEVELS 5
#define BRDF_MAP_RES 512

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <filesystem> 
#include "ResourceManager.h"
#include "Libraries/stb_image_write.h"

class EnvTexturePreprocessor {
public:
	static void init(unsigned int inScreenQuadVAO);
	static void generateIrradianceMap(const std::string& envTexture);
	static void generateSpecularMap(const std::string& envTexture);
	static void generateBRDFIntegrationMap();
	static void writeIrradianceMap(const std::string& folderPath);
	static void writeBRDFIntegrationMap(const std::string& folderPath);
	static unsigned int getIrradianceMap();
	static unsigned int getSpecularMap();
	static unsigned int getBrdfIntegrationMap();
	static unsigned int getRoughnessMipLevels();

private:
	//Private constructor, no instancing
	EnvTexturePreprocessor() {}

	static unsigned int screenQuadVAO;
	static unsigned int cubeVAO;
	static unsigned int cubeNVertices;
	static unsigned int fBO;
	static unsigned int brdfFBO;
	static unsigned int depthRBO;
	
	//
	static unsigned int irradianceMap;
	static unsigned int specularMap;
	static unsigned int brdfIntegrationMap;
};

#endif
#pragma once
