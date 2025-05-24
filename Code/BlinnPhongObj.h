#ifndef BLINN_PHONG_OBJ_H
#define BLINN_PHONG_OBJ_H

#include "ResourceManager.h"
#include "GameObject.h"

class BlinnPhongObj : public GameObject {

public:

	//Creates a gizmo with the given vertex data
	BlinnPhongObj(const std::string& inModel, const glm::vec3& inPosition = glm::vec3(0.0f), const glm::vec3& inScale = glm::vec3(1.0f),
		const glm::vec3& inOrientation = glm::vec3(0.0f), const glm::vec3& inBaseColor = glm::vec3(1.0f), float inSpecular = 0.5f);
	//Renders the blinn-phong gameobject
	void draw() override;
	//Renders the blinn-phong gameobject using the specified shadow map shaderID
	void drawGeometry(unsigned int shaderID) override;

private:
	//The model this gameobject employs
	std::string model;
	// Parameters
	glm::vec3 baseColor;
	float specular;
};

#endif
