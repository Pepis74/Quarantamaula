#ifndef GIZMO_OBJ_H
#define GIZMO_OBJ_H

#include "ResourceManager.h"
#include "GameObject.h"

class GizmoObj : public GameObject {

public:

	//Creates a gizmo with the given vertex data
	GizmoObj(const glm::vec3& inPosition, const glm::vec3& inScale, const glm::vec3& inOrientation,
		const std::vector<glm::vec3>& inVertices, const glm::vec3& inColor);
	//Renders the gizmo
	void draw() override;
	//Renders the gizmo into the shadow map (if for some reason you wanted to idk lol)
	void drawGeometry(unsigned int shaderID) override;
	//Set the gizmo's scale
	void setScale(const glm::vec3& scaler);

private:
	unsigned int vAO;
	std::vector<glm::vec3> vertices;
	glm::vec3 color;
};

#endif
