#include "ShadowMapper.h"

//Initialize static variables
unsigned int ShadowMapper::windowWidth;
unsigned int ShadowMapper::windowHeight;
unsigned int ShadowMapper::debug;
std::vector<GameObject*>* ShadowMapper::shadowCasters;
std::vector<PointLight>* ShadowMapper::ptLights;
DirectionalLight* ShadowMapper::dirLight;
float ShadowMapper::shadowRange; 
unsigned int ShadowMapper::dirShadowRes;
unsigned int ShadowMapper::ptShadowRes;
unsigned int ShadowMapper::dirShadowFBO;
std::vector<unsigned int> ShadowMapper::ptShadowFBOs;
unsigned int ShadowMapper::dirShadowMap;
std::vector<unsigned int> ShadowMapper::ptShadowMaps;
unsigned int ShadowMapper::filterMap;
unsigned int ShadowMapper::filterMapMapSize;
unsigned int ShadowMapper::filterMapFilterSize;
float ShadowMapper::filterMapDirRadius;
float ShadowMapper::filterMapPtRadius;
glm::mat4 ShadowMapper::dirLightSpaceMat;
std::vector<glm::mat4*> ShadowMapper::ptLightSpaceMats;

void ShadowMapper::init(unsigned int inWWidth, unsigned int inWHeight, float inShadowRange, unsigned int inShadowRes,
	unsigned int inFilterMapMapSize, unsigned int inFilterMapFilterSize, std::vector<GameObject*>* inShadowCasters,
	DirectionalLight* inDirLight, std::vector<PointLight>* inPointLights)
{
	//Initialize member variables
	windowWidth = inWWidth;
	windowHeight = inWHeight;
	shadowRange = inShadowRange;
	dirShadowRes = inShadowRes;
	ptShadowRes = inShadowRes / 8;
	shadowCasters = inShadowCasters;
	dirLight = inDirLight;
	ptLights = inPointLights;
	filterMapMapSize = inFilterMapMapSize;
	filterMapFilterSize = inFilterMapFilterSize;
	filterMapDirRadius = 4;
	filterMapPtRadius = 24;

	//Generate the new framebuffer we'll use for directional shadow mapping
	glGenFramebuffers(1, &dirShadowFBO);

	//Generate the directional shadow map texture
	glGenTextures(1, &dirShadowMap);
	glBindTexture(GL_TEXTURE_2D, dirShadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, dirShadowRes, dirShadowRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Attach the texture to the framebuffer we're gonna use to render the directional shadow maps and configure it
	glBindFramebuffer(GL_FRAMEBUFFER, dirShadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dirShadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Compute the directional light space matrix
	glm::mat4 dirLightProjection = glm::ortho(-shadowRange, shadowRange, -shadowRange, shadowRange, 0.1f, shadowRange);
	glm::mat4 dirLightView = glm::lookAt(glm::normalize(-dirLight->direction) * 8.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	dirLightSpaceMat = dirLightProjection * dirLightView;

	//Generate the new framebuffers we'll use for point shadow mapping
	unsigned int fBO;

	for (unsigned int i = 0; i < ptLights->size(); i++) 
	{
		glGenFramebuffers(1, &fBO);
		ptShadowFBOs.push_back(fBO);
	}

	//Generate the point shadow map cubemap textures
	unsigned int cubemap;

	for (unsigned int i = 0; i < ptShadowFBOs.size(); i++)
	{
		glGenTextures(1, &cubemap);
		ptShadowMaps.push_back(cubemap);

		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

		for (unsigned int j = 0; j < 6; j++) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, ptShadowRes, ptShadowRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		//Attach the texture to the framebuffer we're gonna use to render the point shadow maps and configure it
		glBindFramebuffer(GL_FRAMEBUFFER, ptShadowFBOs[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubemap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	//Compute the point light space matrices  
	glm::mat4 ptLightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, shadowRange);

	for (const PointLight& ptLight : *ptLights) {
		glm::mat4* mats = new glm::mat4[6];

		mats[0] = ptLightProjection * glm::lookAt(ptLight.position, ptLight.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
		mats[1] = ptLightProjection * glm::lookAt(ptLight.position, ptLight.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
		mats[2] = ptLightProjection * glm::lookAt(ptLight.position, ptLight.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
		mats[3] = ptLightProjection * glm::lookAt(ptLight.position, ptLight.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
		mats[4] = ptLightProjection * glm::lookAt(ptLight.position, ptLight.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
		mats[5] = ptLightProjection * glm::lookAt(ptLight.position, ptLight.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));

		ptLightSpaceMats.push_back(mats);
	}

	//Create and store the shadow filter map
	filterMap = ShadowFiltersTexture(filterMapMapSize, filterMapFilterSize).getID();
}

void ShadowMapper::render()
{

	//1. Render directional shadow map
	//Change the viewport's resolution so it matches that of the texture we're gonna use to store the directional shadow map
	glViewport(0, 0, dirShadowRes, dirShadowRes);
	//Bind the framebuffer we made 
	glBindFramebuffer(GL_FRAMEBUFFER, dirShadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	//Bind the shader we'll use to render the directional shadow map
	unsigned int shaderID = ResourceManager::getShader("dirShadow")->getID();
	glUseProgram(shaderID);

	//Draw regular gameObjects
	for (auto& object : *shadowCasters) { object->drawGeometry(shaderID); }

	//2. Render point shadow map
	//Change the viewport's resolution so it matches that of the texture we're gonna use to store the point shadow map
	glCullFace(GL_FRONT);
	glViewport(0, 0, ptShadowRes, ptShadowRes);
	//Bind the shader we'll use to render the point shadow map
	shaderID = ResourceManager::getShader("ptShadow")->getID();
	glUseProgram(shaderID);

	//Query the lightSpaceMats uniform location
	unsigned int matsLoc = glGetUniformLocation(shaderID, "ptLightSpaceMats");

	//For each point light
	for (unsigned int i = 0; i < ptLights->size(); i++) 
	{
		//Bind the corresponding framebuffer we made 
		glBindFramebuffer(GL_FRAMEBUFFER, ptShadowFBOs[i]);
		glClear(GL_DEPTH_BUFFER_BIT);

		//Send light space matrices to the shader
		glUniformMatrix4fv(matsLoc, 6, false, glm::value_ptr(*ptLightSpaceMats[i]));

		//Send the point light's position to the shader
		glUniform1ui(glGetUniformLocation(shaderID, "lightIndex"), i);

		//Draw regular gameObjects
		for (auto& object : *shadowCasters) { object->drawGeometry(shaderID); }
	}

	//Restore the initial viewport and framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
	glViewport(0, 0, windowWidth, windowHeight);
}

void ShadowMapper::updateShadowUniformBlocks(unsigned int* uBOs)
{
	//Point Lights
	glBindBuffer(GL_UNIFORM_BUFFER, uBOs[2]);
	glBufferSubData(GL_UNIFORM_BUFFER, 244, sizeof(unsigned int), &shadowRange);

	//Lighting Misc
	glBindBuffer(GL_UNIFORM_BUFFER, uBOs[3]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(dirLightSpaceMat));

	//Filter Map Params
	glBindBuffer(GL_UNIFORM_BUFFER, uBOs[4]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(unsigned int), &filterMapMapSize);
	glBufferSubData(GL_UNIFORM_BUFFER, 4, sizeof(unsigned int), &filterMapFilterSize);
	glBufferSubData(GL_UNIFORM_BUFFER, 8, sizeof(float), &filterMapDirRadius);
	glBufferSubData(GL_UNIFORM_BUFFER, 12, sizeof(float), &filterMapPtRadius);
}

void ShadowMapper::setShadowTextures(unsigned int shaderID, unsigned int gNum)
{
	//Set the directional shadow map texture in location NUM_G_BUFFER_TEXTURES + 5
	QUtils::bindTexture2D(shaderID, "dirShadowMap", gNum + 5, dirShadowMap);

	//Set the point light shadow map textures in location NUM_G_BUFFER_TEXTURES + 6 onwards
	for (unsigned int i = 0; i < MAX_POINT_LIGHTS; i++) 
	{
		glUniform1i(glGetUniformLocation(shaderID, ("ptShadowMaps[" + std::to_string(i) + "]").c_str()), i + gNum + 6);

		if (i < ptLights->size()) 
		{
			glActiveTexture(GL_TEXTURE0 + i + gNum + 6);
			glBindTexture(GL_TEXTURE_CUBE_MAP, ptShadowMaps[i]);
		}
	}

	//Set the shadow filters texture in location NUM_G_BUFFER_TEXTURES + 4
	QUtils::bindTexture3D(shaderID, "filterMap", gNum + 4, filterMap);
}
