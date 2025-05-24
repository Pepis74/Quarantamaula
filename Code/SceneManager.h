#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#define NUM_SHADERS 3
#define NUM_UNIFORM_BLOCKS 6
#define NUM_G_BUFFER_TEXTURES 5
#define EXPOSURE 2
#define GAMMA 2.2f

#include "Camera.h"
#include "BlinnPhongObj.h"
#include "GizmoObj.h"
#include "ToonObj.h"
#include "PBRObj.h"
#include "Skybox.h"
#include "ShadowMapper.h"
#include "QUtils.h"
#include "EnvTexturePreprocessor.h"

class SceneManager {

public:

	//Initialize the game manager and set the inital game scene
	static void init(unsigned int inWWidth, unsigned int inWHeight, unsigned int inShadowRange, unsigned int inShadowRes, Camera* inCamera);
	//Draw all the objects on the scene
	static void render();
	//Toggle the gizmos boolean
	static void toggleGizmos();
	//Increments the show index value 
	static void incrementShowIndex();

private:

	//Prepares the scene's lighting and lighting gizmos
	static void setUpLighting();
	//Prepares the necessary screenquad, framebuffers and textures for deferred shading
	static void setUpDeferredShading();
	//Prepares the necessary screenquad, framebuffers and textures for deferred shading
	static void setUpGameObjects();
	//
	static void updateUniformBlocks();
	//Draw the gizmos
	static void gizmoPass();

	//Private constructor, no instancing
	SceneManager() {};
	//The width and height of the window
	static unsigned int windowWidth, windowHeight;
	//The framebuffer we render to during the geometry pass
	static unsigned int gBuffer;
	//The textures rendered during the geometry pass used as input for the lighting pass. 0 - position/roughness, 1 - normals/specular,
	// 2 - tangents, 3 - albedo/anisotropic, 4 - metallic/clearcoat/clearcoatRoughness
	static unsigned int gBufferTextures[NUM_G_BUFFER_TEXTURES];
	//The shader mask buffers for each shader. 0 R - Blinn-Phong, 1 G - Toon, 2 B - PBR
	static unsigned int shaderMaskBuffers[1];
	//The shader mask textures for each shader.  0 R - Blinn-Phong, 1 G - Toon, 2 B - PBRs
	static unsigned int shaderMaskTextures[1];
	//
	static unsigned int screenQuadVAO;
	//The IDs of the UBOs for each uniform block
	static unsigned int uBOs[NUM_UNIFORM_BLOCKS];
	//Vector of all the gameObjects in the scene that are rendered normally using deferred shading
	static std::vector<GameObject*> regularGameObjects;
	//Vector of all the gameObjects in the scene that cast shadows
	static std::vector<GameObject*> shadowCastingGameObjects;
	//Array of vectors of gameObjects all rendered with the same lit shader. Used to separate gameObjects based on shader
	static std::vector<GameObject*> shaderGameObjects[NUM_SHADERS];
	//True if gizmos are to be rendered, false otherwise
	static bool showGizmos;
	//Position in the gizmos vector where the first of the 3 consecutively stored axis gizmos is
	static unsigned int axisGizmosPos;
	// Index that determines what to show on screen. 0 - regular render, 1 - G Position, 2 - G Normals, 3 - G Albedo, 4 - G Roughness,
    // 5 - G Specular, 6 - G Metallic, 7 - G Anisotropic, 8 - G Clearcoat, 9 - G ClearcoatRoughness, 10 - Shader Mask
	static unsigned int showIndex;
	//Vector of all the gameObjects in the scene
	static std::vector<PointLight> ptLights;
	//Directional light in the scene
	static DirectionalLight dirLight;
	//Camera used as POV
	static Camera* camera;
	//Projection matrix used to render the scene
	static glm::mat4 proj;
	//Skybox gameobject
	static Skybox skybox;

	//Vectors of gameObject derived classes
	//Vector of all the objects rendered via blinn-phong
	static std::vector<BlinnPhongObj> blinnPhongs;
	//Vector of all the gizmos in the scene
	static std::vector<GizmoObj> gizmos;
	//Vector of all the objects rendered via toon shading
	static std::vector<ToonObj> toons;
	//Vector of all the objects rendered via PBR
	static std::vector<PBRObj> pbrs;
};

#endif 
