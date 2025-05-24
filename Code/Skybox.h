#ifndef SKYBOX_H
#define SKYBOX_H

#include "ResourceManager.h"
#include "GameObject.h"
#include "EnvTexturePreprocessor.h"

class Skybox : public GameObject {

public:

	//Creates a gizmo with the given vertex data
	Skybox(const std::string inTexture, const glm::vec3& inPosition = glm::vec3(0.0f), const glm::vec3& inScale = glm::vec3(1.0f),
		const glm::vec3& inOrientation = glm::vec3(0.0f));
	Skybox() : GameObject() {};
	//Renders the skybox
	void draw() override;
	//Renders the cube geometry into the shadow map if you wish baby 
	void drawGeometry(unsigned int shaderID) override;

private:
	// Vertex Array Object of the cube mesh
	unsigned int vAO;
	// Numver of vertices of the cube mesh
	unsigned int nVertices;
	//The skybox's cubemap texture
	std::string texture;
};

#endif
