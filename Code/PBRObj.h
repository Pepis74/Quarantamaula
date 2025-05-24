#ifndef PBR_OBJ_H
#define PBR_OBJ_H

#include "ResourceManager.h"
#include "GameObject.h"

class PBRObj : public GameObject {

public:

	//Creates a gizmo with the given vertex data
	PBRObj(const std::string& inModel, const glm::vec3& inPosition = glm::vec3(0.0f), const glm::vec3& inScale = glm::vec3(1.0f), const glm::vec3& inOrientation = glm::vec3(0.0f),
		const glm::vec3& inBaseColor = glm::vec3(1.0f), float inRoughness = 0.5f, float inSpecular = 0.5f,
		float inMetallic = 0.0f, float inAnisotropic = 0.0f, float inClearcoat = 0.0f, float inClearcoatRoughness = 0.1f);
	//Renders the blinn-phong gameobject
	void draw() override;
	//Renders the blinn-phong gameobject using the specified shadow map shaderID
	void drawGeometry(unsigned int shaderID) override;

private:
	//The model this gameobject employs
	std::string model;

	//Parameters
	glm::vec3 baseColor;
	float roughness;
	float specular;
	float metallic;
	float anisotropic;
	float clearcoat;
	float clearcoatRoughness;
};

#endif
