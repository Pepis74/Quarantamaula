#ifndef TOON_OBJ_H
#define TOON_OBJ_H

#include "ResourceManager.h"
#include "GameObject.h"

class ToonObj : public GameObject {

public:

	//Creates a gizmo with the given vertex data
	ToonObj(const std::string& inModel, const glm::vec3& inPosition, const glm::vec3& inScale, const glm::vec3& inOrientation, const glm::vec3& inBaseColor);
	//Renders the blinn-phong gameobject
	void draw() override;
	//Renders the blinn-phong gameobject using the specified shadow map shaderID
	void drawGeometry(unsigned int shaderID) override;

private:
	//The model this gameobject employs
	std::string model;
	//
	glm::vec3 baseColor;
};

#endif
