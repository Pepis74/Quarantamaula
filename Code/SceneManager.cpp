#include "SceneManager.h"

//#define WRITE_IRRADIANCE_MAP
//#define WRITE_BRDF_MAP

//Instantiate static variables
unsigned int SceneManager::windowWidth;
unsigned int SceneManager::windowHeight;
unsigned int SceneManager::gBuffer;
unsigned int SceneManager::gBufferTextures[NUM_G_BUFFER_TEXTURES];
unsigned int SceneManager::shaderMaskBuffers[1];
unsigned int SceneManager::shaderMaskTextures[1];
unsigned int SceneManager::uBOs[NUM_UNIFORM_BLOCKS];
unsigned int SceneManager::screenQuadVAO;
std::vector<GameObject*> SceneManager::regularGameObjects;
std::vector<GameObject*> SceneManager::shadowCastingGameObjects;
std::vector<GameObject*> SceneManager::shaderGameObjects[NUM_SHADERS];
bool SceneManager::showGizmos;
unsigned int SceneManager::axisGizmosPos;
unsigned int SceneManager::showIndex;
std::vector<PointLight> SceneManager::ptLights;
DirectionalLight SceneManager::dirLight;
Camera* SceneManager::camera;
glm::mat4 SceneManager::proj;
Skybox SceneManager::skybox;

std::vector<BlinnPhongObj> SceneManager::blinnPhongs;
std::vector<GizmoObj> SceneManager::gizmos;
std::vector<ToonObj> SceneManager::toons;
std::vector<PBRObj> SceneManager::pbrs;

void SceneManager::init(unsigned int inWWidth, unsigned int inWHeight, unsigned int inShadowRange, unsigned int inShadowRes, Camera* inCamera)
{
	showGizmos = false;
	
	//Initialize member variables
	windowWidth = inWWidth;
	windowHeight = inWHeight;
	camera = inCamera;

	//Load the resources
	ResourceManager::loadShader("Assets/Shaders/LitPassCommon.vs", "Assets/Shaders/PBR.fs", "pbr");
	ResourceManager::loadShader("Assets/Shaders/LitPassCommon.vs", "Assets/Shaders/Toon.fs", "toon");
	ResourceManager::loadShader("Assets/Shaders/LitPassCommon.vs", "Assets/Shaders/SkyboxLighting.fs", "skyboxLighting");
	ResourceManager::loadShader("Assets/Shaders/SkyboxGeometry/SkyboxGeometry.vs", "Assets/Shaders/SkyboxGeometry/SkyboxGeometry.fs", "skyboxGeometry");
	ResourceManager::loadShader("Assets/Shaders/Gizmo/Gizmo.vs", "Assets/Shaders/Gizmo/Gizmo.fs", "gizmo");
	ResourceManager::loadShader("Assets/Shaders/DirShadow/DirShadow.vs", "Assets/Shaders/DirShadow/DirShadow.fs", "dirShadow");
	ResourceManager::loadShader("Assets/Shaders/PtShadow/PtShadow.vs", "Assets/Shaders/PtShadow/PtShadow.gs", "Assets/Shaders/PtShadow/PtShadow.fs", "ptShadow");
	ResourceManager::loadShader("Assets/Shaders/GeometryPass/GeometryPass.vs", "Assets/Shaders/GeometryPass/GeometryPass.fs", "geometryPass");
	ResourceManager::loadShader("Assets/Shaders/ShaderMask/ShaderMask.vs", "Assets/Shaders/ShaderMask/ShaderMask.fs", "shaderMask");
	ResourceManager::loadShader("Assets/Shaders/EnvCubemap/EnvCubemap.vs", "Assets/Shaders/EnvCubemap/IrradianceMap.fs", "irradianceMap");
	ResourceManager::loadShader("Assets/Shaders/EnvCubemap/EnvCubemap.vs", "Assets/Shaders/EnvCubemap/SpecularMap.fs", "specularMap");
	ResourceManager::loadShader("Assets/Shaders/LitPassCommon.vs", "Assets/Shaders/BRDFIntegrator.fs", "brdfIntegrator");
	ResourceManager::loadModel("Assets/Models/Plane/plane.obj", "plane");
	ResourceManager::loadModel("Assets/Models/ChessBoard/Untitled.obj", "chess");
	ResourceManager::loadModel("Assets/Models/Checker/Black.obj", "blackChecker");
	ResourceManager::loadModel("Assets/Models/Checker/White.obj", "whiteChecker");
	ResourceManager::loadModel("Assets/Models/Arrow/Arrow.obj", "arrow");
	ResourceManager::loadModel("Assets/Models/Sphere/sphere.obj", "sphere");
	ResourceManager::loadEnvCubemap("Assets/HighResEnv", "hiRes", false);
	ResourceManager::loadEnvCubemap("Assets/LowResEnv", "lowRes", false);
	ResourceManager::loadEnvCubemap("Assets/IrradianceMap", "irradianceMap", false);
	ResourceManager::loadEnvTexture("Assets/BRDFIntegrationMap.hdr", "brdfIntegrationMap", false);
	
	//
	setUpDeferredShading();

	setUpGameObjects();
	
	//Set up the uniform buffer objects and bind them to their corresponding binding points
	unsigned int uniformBlockSizes[NUM_UNIFORM_BLOCKS] = { 128, 32, 256, 80, 16, 16 };

	for (unsigned int i = 0; i < NUM_UNIFORM_BLOCKS; i++) 
	{
		glGenBuffers(1, &uBOs[i]);
		glBindBuffer(GL_UNIFORM_BUFFER, uBOs[i]);
		glBufferData(GL_UNIFORM_BUFFER, uniformBlockSizes[i], NULL, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, i, uBOs[i]);
	}
	
	//Set up the projection matrix we're gonna use
	proj = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

	//Prepares the scene's lighting and lighting gizmos
	setUpLighting();

	//
	ShadowMapper::init(windowWidth, windowHeight, inShadowRange, inShadowRes, 8, 8, &shadowCastingGameObjects, &dirLight, &ptLights);

	EnvTexturePreprocessor::init(screenQuadVAO);
	EnvTexturePreprocessor::generateSpecularMap("lowRes");

#ifdef WRITE_IRRADIANCE_MAP
	EnvTexturePreprocessor::generateIrradianceMap("lowRes");
	EnvTexturePreprocessor::writeIrradianceMap("C:\\Users\\sergio\\Documents\\Quarantamaula\\Assets\\IrradianceMap");
#endif

#ifdef WRITE_BRDF_MAP
	EnvTexturePreprocessor::generateBRDFIntegrationMap();
	EnvTexturePreprocessor::writeBRDFIntegrationMap("C:\\Users\\sergio\\Documents\\Quarantamaula\\Assets");
#endif 

	//Restore viewport after rendering the environment textures
	glViewport(0, 0, windowWidth, windowHeight);

	//Save the position where the first of the 3 consecutively stored axis gizmos will be
	axisGizmosPos = gizmos.size();

	//Create gizmos we'll use to show the xyz axes
	for (unsigned int i = 0; i < 3; i++) {
		std::vector<glm::vec3> axisVertices;
		glm::vec3 firstVertex = glm::vec3(0.0f);
		glm::vec3 secondVertex = glm::vec3(0.0f);
		firstVertex[i] = 1.0f;
		secondVertex[i] = -1.0f;
		axisVertices.push_back(firstVertex);
		axisVertices.push_back(secondVertex);
		glm::vec3 axisColor = glm::vec3(0.0f);
		axisColor[i] = 1.0f;
		gizmos.push_back(GizmoObj(glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), axisVertices, axisColor));
	}
}

void SceneManager::setUpLighting()
{
	//Configure directional light
	dirLight.direction = glm::vec3(-0.5, -0.17, -0.7);
	dirLight.color = glm::vec3(1.0f, 0.84f, 0.5f);
	dirLight.intensity = 1.0f;

	//Create directional light gizmo
	std::vector<glm::vec3> dirLightVertices;
	dirLightVertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	dirLightVertices.push_back(glm::normalize(dirLight.direction));

	gizmos.push_back(GizmoObj(glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), dirLightVertices, dirLight.color));

	//Configure point lights and create point light gizmos
	std::vector<glm::vec3> ptLightVertices;
	ptLightVertices.push_back(glm::vec3(0.1f, 0.0f, 0.0f));
	ptLightVertices.push_back(glm::vec3(-0.1f, 0.0f, 0.0f));
	ptLightVertices.push_back(glm::vec3(0.0f, 0.1f, 0.0f));
	ptLightVertices.push_back(glm::vec3(0.0f, -0.1f, 0.0f));
	ptLightVertices.push_back(glm::vec3(0.0f, 0.0f, 0.1f));
	ptLightVertices.push_back(glm::vec3(0.0f, 0.0f, -0.1f));
	
	
	/*PointLight p1;
	p1.position = glm::vec3(0.0f, 3.0f, 0.0f);
	p1.color = glm::vec3(1.0f);
	p1.intensity = 1.0f;
	ptLights.push_back(p1);
	gizmos.push_back(GizmoObj(p1.position, glm::vec3(1.0f), glm::vec3(0.0f), ptLightVertices, p1.color));*/
}

void SceneManager::setUpDeferredShading()
{
	//Create the vertex data for the screen quad
	unsigned int vBO;

	float vertices[] =
	{
		//Position  //Texture coordinates
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f
	};

	//Create VAO and VBO
	glGenVertexArrays(1, &screenQuadVAO);
	glGenBuffers(1, &vBO);
	glBindVertexArray(screenQuadVAO);

	//Configure VBO
	glBindBuffer(GL_ARRAY_BUFFER, vBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Configure VAO
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	//Unbind VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Set up the G buffer for deferred rendering
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	//Set up g buffer textures
	for (unsigned int i = 0; i < NUM_G_BUFFER_TEXTURES; i++) 
	{
		glGenTextures(1, &gBufferTextures[i]);
		glBindTexture(GL_TEXTURE_2D, gBufferTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, gBufferTextures[i], 0);
	}

	//Tell OpenGL how many targets to render to for the G buffer
	unsigned int gAttachments[NUM_G_BUFFER_TEXTURES];

	for (unsigned int i = 0; i < NUM_G_BUFFER_TEXTURES; i++)
	{
		gAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
	}

	glDrawBuffers(NUM_G_BUFFER_TEXTURES, gAttachments);

	//Set up and attach a depth buffer to the G framebuffer
	unsigned int gBufferDepth;
	glGenRenderbuffers(1, &gBufferDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, gBufferDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gBufferDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "G buffer framebuffer not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Set up the shader mask buffers to render masks to differentiate objects based on the shader they're being rendered with during the
	//lighting pass
	unsigned int shaderMaskDepth;

	for (unsigned int i = 0; i < std::ceil(NUM_SHADERS / 4.0) ; i++)
	{
		glGenFramebuffers(1, &shaderMaskBuffers[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, shaderMaskBuffers[i]);
		glGenTextures(1, &shaderMaskTextures[i]);
		glBindTexture(GL_TEXTURE_2D, shaderMaskTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, shaderMaskTextures[i], 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glGenRenderbuffers(1, &shaderMaskDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, shaderMaskDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, shaderMaskDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Shader mask framebuffer not complete!" << std::endl;
	}

	//Rebind the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneManager::setUpGameObjects()
{
	//Blinn Phongs
	//blinnPhongs.push_back(BlinnPhongObj("backpack", glm::vec3(-1.0f, 0.82f, 0.2f), glm::vec3(0.5f)));
	//blinnPhongs.push_back(BlinnPhongObj(glm::vec3(3.0f, 1.0f, 3.0f), glm::vec3(1.0f), glm::vec3(0.0f), "cube"));
	//blinnPhongs.push_back(BlinnPhongObj(glm::vec3(-3.0f, 1.0f, -3.0f), glm::vec3(1.0f), glm::vec3(0.0f), "cube"));
	//blinnPhongs.push_back(BlinnPhongObj(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(5.0f), glm::vec3(0.0f), "plane"));
	//pbrs.push_back(PBRObj(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f), "sphere"));
	//blinnPhongs.push_back(BlinnPhongObj(glm::vec3(-1.0f, 0.82f, 0.2f), glm::vec3(0.5f), glm::vec3(0.0f), "sphere"));

	//Toons
	//toons.push_back(ToonObj(glm::vec3(0.6f, 0.0f, 0.6f), glm::vec3(0.01f), glm::vec3(0.0f), "teapot"));

	//PBRs
	pbrs.push_back(PBRObj("chess", glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(5.0f), 0.9f, 0.6f, 0.0f, 0.0f, 0.0f, 0.1f));
	pbrs.push_back(PBRObj("blackChecker", glm::vec3(2.2f, 0.2f, -0.32f), glm::vec3(0.35f), glm::vec3(0.0f), glm::vec3(0.7f, 0.7f, 0.7f), 0.9f, 0.5f, 0.0f, 0.0f, 1.0f, 0.05f));
	pbrs.push_back(PBRObj("whiteChecker", glm::vec3(2.2f, 0.2f, 0.32f), glm::vec3(0.35f), glm::vec3(0.0f), glm::vec3(0.7f, 0.7f, 0.7f), 0.9f, 0.5f, 0.0f, 0.0f, 1.0f, 0.05f));
	//toons.push_back(ToonObj("arrow", glm::vec3(2.2f, 0.9f, 0.32f), glm::vec3(0.1f), glm::vec3(-90.0f, 0.0f, 0.0f), glm::vec3(0.9f, 0.0f, 0.0f)));
	//pbrs.push_back(PBRObj("sphere", glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f));

	//Set up regular gameobjects and shadow casters
	for (unsigned int i = 0 ; i < blinnPhongs.size(); i++)
	{
		shaderGameObjects[0].push_back(&blinnPhongs[i]);
		regularGameObjects.push_back(&blinnPhongs[i]);
		shadowCastingGameObjects.push_back(&blinnPhongs[i]);
	}

	for (unsigned int i = 0; i < toons.size(); i++)
	{
		shaderGameObjects[1].push_back(&toons[i]);
		regularGameObjects.push_back(&toons[i]);
	}

	for (unsigned int i = 0; i < pbrs.size(); i++)
	{
		shaderGameObjects[2].push_back(&pbrs[i]);
		regularGameObjects.push_back(&pbrs[i]);
		shadowCastingGameObjects.push_back(&pbrs[i]);
	}

	//Set up the skybox
	skybox = Skybox("hiRes", glm::vec3(0.0f), glm::vec3(25.0f));
}

void SceneManager::render()
{
	//Update uniform blocks before rendering 
	updateUniformBlocks();

	//Shadow map pass
	ShadowMapper::render();

	//Disable blending before doing any deferred shading
	glDisable(GL_BLEND);

	//Geometry pass
	//Bind the G Buffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Render the skybox
	unsigned int shaderID = ResourceManager::getShader("skyboxGeometry")->getID();
	glUseProgram(shaderID);
	glDepthMask(GL_FALSE);
	skybox.draw();
	glDepthMask(GL_TRUE);

	//Use the geometry pass shader
	shaderID = ResourceManager::getShader("geometryPass")->getID();
	glUseProgram(shaderID);
	
	//Draw gameObjects to fill the g buffer
	for (const auto& obj : regularGameObjects)
	{
		obj->draw();
	}

	//Then render each object with a different lit shader separately using the shader mask shader to get the masks used to render each 
	//object with the appropiate lit shader during the lighting pass
	shaderID = ResourceManager::getShader("shaderMask")->getID();
	glUseProgram(shaderID);

	//Each shader mask is stored in a different channel of RGBA textures
	for (unsigned int i = 0; i < NUM_SHADERS; i++) 
	{
		//One-hot vector that determines the channel the shader mask will be stored in
		glm::vec4 channel;

		switch (i % 4) 
		{
			case 0:
				channel = glm::vec4(1.0, 0.0, 0.0, 0.0);

				//If we've filled out the texture's channels, we bind the next framebuffer with a new texture attached to it
				glBindFramebuffer(GL_FRAMEBUFFER, shaderMaskBuffers[static_cast<int>(std::floor(i / 4.0))]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				break;

			case 1:
				channel = glm::vec4(0.0, 1.0, 0.0, 0.0);
				break;

			case 2:
				channel = glm::vec4(0.0, 0.0, 1.0, 0.0);
				break;

			case 3:
				channel = glm::vec4(0.0, 0.0, 0.0, 1.0);
				break;
		}

		//Send the texture to the shader
		glUniform4fv(glGetUniformLocation(shaderID, "channel"), 1, glm::value_ptr(channel));

		for (auto& obj : shaderGameObjects[i]) 
		{
			obj->drawGeometry(shaderID);
		}
	}
	
	//Lighting pass
	//Bind the default framebuffer to render to screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//Disable face culling and depth testing to be able to see the screen quad
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//Clear the buffer bits
	glClear(GL_COLOR_BUFFER_BIT);
	//Reenable blending to blend the different screen quads of the different shaders
	glEnable(GL_BLEND);

	//Skybox
	shaderID = ResourceManager::getShader("skyboxLighting")->getID();
	glUseProgram(shaderID);
	
	QUtils::bindTexture2D(shaderID, "gAlbedoAnisotropic", 3, gBufferTextures[3]);

	//Bind the VAO and make draw the screen quad
	glBindVertexArray(screenQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	/*//Blinn Phong
	shaderID = ResourceManager::getShader("blinnPhong")->getID();
	glUseProgram(shaderID);

	//Set g buffer texture units in the shader
	QUtils::bindTexture2D(shaderID, "gPositionRoughness", 0, gBufferTextures[0]);
	QUtils::bindTexture2D(shaderID, "gNormalsSpecular", 1, gBufferTextures[1]);
	QUtils::bindTexture2D(shaderID, "gAlbedoAnisotropic", 3, gBufferTextures[3]);
	//Set appropriate shader mask texture
	QUtils::bindTexture2D(shaderID, "shaderMask", NUM_G_BUFFER_TEXTURES, shaderMaskTextures[0]);

	//Set the shadow map textures
	ShadowMapper::setShadowTextures(shaderID, NUM_G_BUFFER_TEXTURES);

	//Bind the VAO and make draw the screen quad
	glBindVertexArray(screenQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);*/

	//Toon
	shaderID = ResourceManager::getShader("toon")->getID();
	glUseProgram(shaderID);

	//Set g buffer texture units in the shader
	QUtils::bindTexture2D(shaderID, "gAlbedoAnisotropic", 3, gBufferTextures[3]);
	//Set appropriate shader mask texture
	QUtils::bindTexture2D(shaderID, "shaderMask", NUM_G_BUFFER_TEXTURES, shaderMaskTextures[0]);

	//Bind the VAO and make draw the screen quad
	glBindVertexArray(screenQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	//PBR
	shaderID = ResourceManager::getShader("pbr")->getID();
	glUseProgram(shaderID);

	//Bind the environment map textures
	QUtils::bindTextureCubemap(shaderID, "irradianceMap", NUM_G_BUFFER_TEXTURES + 1, ResourceManager::getEnvTexture("irradianceMap"));
	QUtils::bindTextureCubemap(shaderID, "specularMap", NUM_G_BUFFER_TEXTURES + 2, EnvTexturePreprocessor::getSpecularMap());
	QUtils::bindTexture2D(shaderID, "brdfIntegrationMap", NUM_G_BUFFER_TEXTURES + 3, ResourceManager::getEnvTexture("brdfIntegrationMap"));

	//Set g buffer texture units in the shader
	QUtils::bindTexture2D(shaderID, "gPositionRoughness", 0, gBufferTextures[0]);
	QUtils::bindTexture2D(shaderID, "gNormalsSpecular", 1, gBufferTextures[1]);
	QUtils::bindTexture2D(shaderID, "gTangents", 2, gBufferTextures[2]);
	QUtils::bindTexture2D(shaderID, "gAlbedoAnisotropic", 3, gBufferTextures[3]);
	QUtils::bindTexture2D(shaderID, "gMetallicClearcoatClearcoatroughness", 4, gBufferTextures[4]);
	//Set appropriate shader mask texture
	QUtils::bindTexture2D(shaderID, "shaderMask", NUM_G_BUFFER_TEXTURES, shaderMaskTextures[0]);

	//Set the shadow map textures
	ShadowMapper::setShadowTextures(shaderID, NUM_G_BUFFER_TEXTURES);

	//Bind the VAO and make draw the screen quad
	glBindVertexArray(screenQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	//Reenable face culling once we're done rendering the quads of the deferred pass
	glEnable(GL_CULL_FACE);

	//Gizmo pass
	if (showGizmos) 
	{
		//Scale the axis gizmos properly
		float cameraDistance = glm::distance(camera->Position, glm::vec3(0.0f));
		
		for (unsigned int i = 0; i < 3; i++) {
			glm::vec3 scaler = glm::vec3(0.0f);
			scaler[i] = 1.0f;
			gizmos[axisGizmosPos + i].setScale(cameraDistance * 1.0f * scaler);
		}

		//Draw the gizmos
		gizmoPass();
	}

	//Reenable depth testing after gizmos have been drawn
	glEnable(GL_DEPTH_TEST);
}

void SceneManager::updateUniformBlocks()
{
	//VPMatrices
	glBindBuffer(GL_UNIFORM_BUFFER, uBOs[0]);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera->GetViewMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, 64, sizeof(glm::mat4), glm::value_ptr(proj));

	//Directional Light
	glBindBuffer(GL_UNIFORM_BUFFER, uBOs[1]);

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec3), glm::value_ptr(dirLight.direction));
	glBufferSubData(GL_UNIFORM_BUFFER, 12, sizeof(float), &dirLight.intensity);
	glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(glm::vec3), glm::value_ptr(dirLight.color));

	//Point Lights. The far plane is set later in the Shadow Mapper
	glBindBuffer(GL_UNIFORM_BUFFER, uBOs[2]);

	for (unsigned int i = 0; i < ptLights.size(); i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, 0 + i * sizeof(glm::vec4), sizeof(glm::vec3), glm::value_ptr(ptLights[i].position));
		glBufferSubData(GL_UNIFORM_BUFFER, 80 + i * sizeof(glm::vec4), sizeof(glm::vec3), glm::value_ptr(ptLights[i].color));
		glBufferSubData(GL_UNIFORM_BUFFER, 160 + i * sizeof(glm::vec4), sizeof(float), &ptLights[i].intensity);
	}

	unsigned int nPtLights = ptLights.size();
	glBufferSubData(GL_UNIFORM_BUFFER, 240, sizeof(unsigned int), &nPtLights);

	//Lighting Misc. The dirLightSpaceMat is set in the Shadow Mapper
	glBindBuffer(GL_UNIFORM_BUFFER, uBOs[3]);
	glBufferSubData(GL_UNIFORM_BUFFER, 64, sizeof(glm::vec3), glm::value_ptr(camera->Position));

	//Filter Map Params will be set entirely in the Shadow Mapper
	ShadowMapper::updateShadowUniformBlocks(uBOs);

	//Lighting Pass Common
	glBindBuffer(GL_UNIFORM_BUFFER, uBOs[5]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(unsigned int), &showIndex);

	float exposure = EXPOSURE;
	glBufferSubData(GL_UNIFORM_BUFFER, 4, sizeof(float), &exposure);

	float gamma = GAMMA;
	glBufferSubData(GL_UNIFORM_BUFFER, 8, sizeof(float), &gamma);

	// Minus one because mip levels start al 0
	unsigned int roughnessMipLevels = EnvTexturePreprocessor::getRoughnessMipLevels() - 1;
	glBufferSubData(GL_UNIFORM_BUFFER, 12, sizeof(unsigned int), &roughnessMipLevels);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SceneManager::gizmoPass()
{
	//Get and set the gizmo shader
	unsigned int shaderID = ResourceManager::getShader("gizmo")->getID();
	glUseProgram(shaderID);

	for (GizmoObj gizmo : gizmos)
	{
		gizmo.draw();
	}
}

//Toggle the gizmos boolean
void SceneManager::toggleGizmos()
{
	if (showGizmos) showGizmos = false;
	else showGizmos = true;
}

void SceneManager::incrementShowIndex()
{
	showIndex++;

	if (showIndex > 11) showIndex = 0;
}
