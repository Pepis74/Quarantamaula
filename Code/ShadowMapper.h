#ifndef SHADOW_MAPPER_H
#define SHADOW_MAPPER_H

#include "Camera.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "ShadowFiltersTexture.h"
#include "QUtils.h"

struct DirectionalLight {
	//The light's direction
	glm::vec3 direction;
	//The light's color
	glm::vec3 color;
	//The light's intensity
	float intensity;
};

struct PointLight {
	//The light's position
	glm::vec3 position;
	//The light's color
	glm::vec3 color;
	//The light's intensity
	float intensity;
};

class ShadowMapper{

public:

	//Initialize the shadow mapper
	static void init(unsigned int inWWidth, unsigned int inWHeight, float inShadowRange, unsigned int inShadowRes, 
		unsigned int inFilterMapMapSize, unsigned int inFilterMapFilterSize, std::vector<GameObject*>* inShadowCasters, 
		DirectionalLight* inDirLight, std::vector<PointLight>* inPointLights);
	//Render the shadow map
	static void render();
	//Updates the corresponding part of uniform blocks
	static void updateShadowUniformBlocks(unsigned int* uBOs);
	//Sends shadow information to the shader passed as an argument
	static void setShadowTextures(unsigned int shaderID, unsigned int gNum);

private:


	//Private constructor, no instancing
	ShadowMapper() {};
	//The width and height of the window
	static unsigned int windowWidth, windowHeight;
	static unsigned int debug;
	//Vector of all the gameObjects in the scene
	static std::vector<GameObject*>* shadowCasters;
	//Vector of all the gameObjects in the scene
	static std::vector<PointLight>* ptLights;
	//Directional light in the scene
	static DirectionalLight* dirLight;
	//Range in which shadows are casted
	static float shadowRange;
	//Framebuffer used to render the shadow map
	static unsigned int dirShadowRes;
	//Framebuffer used to render the shadow map
	static unsigned int ptShadowRes;
	//Framebuffer used to render the shadow map
	static unsigned int dirShadowFBO;
	//Framebuffer used to render the shadow map
	static std::vector<unsigned int> ptShadowFBOs;
	//Texture where we store the shadow map
	static unsigned int dirShadowMap;
	//Texture where we store the shadow map
	static std::vector<unsigned int> ptShadowMaps;
	//Texture where we store the shadow map filters
	static unsigned int filterMap;
	// 
	static unsigned int filterMapMapSize;
	//
	static unsigned int filterMapFilterSize;
	//
	static float filterMapDirRadius;
	//
	static float filterMapPtRadius;
	// 
	//The matrix used to transform a vertex so that it's seen from the directional light's POV
	static glm::mat4 dirLightSpaceMat;
	//
	static std::vector<glm::mat4*> ptLightSpaceMats;
};

#endif 
