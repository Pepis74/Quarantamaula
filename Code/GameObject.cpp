#include "GameObject.h"

GameObject::GameObject(const glm::vec3& inPosition, const glm::vec3& inScale, const glm::vec3& inOrientation) 
: position(inPosition), scale(inScale), orientation(inOrientation) {}

glm::mat4 GameObject::computeModelMat()
{
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotation = glm::mat4_cast(glm::quat(glm::radians(orientation)));
	glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);
	return (translation * rotation * scaling);
}