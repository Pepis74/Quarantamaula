#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class GameObject {

public:
	//A gameObject is anything situated in the game world that can be drawn.
	GameObject(const glm::vec3& inPosition, const glm::vec3& inScale, const glm::vec3& inOrientation);
	GameObject() : position(glm::vec3(0.0f)), scale(glm::vec3(1.0f)), orientation(glm::vec3(0.0f)) {}
	//Abstract draw function
	virtual void draw() = 0;
	//Abstract function to draw just the geometry of the vertices attached to the gameObject. Used for shadow maps, shader masks, etc
	virtual void drawGeometry(unsigned int shaderID) = 0;

protected:
	//Computes the model matrix
	glm::mat4 computeModelMat();

	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 orientation;
};

#endif
