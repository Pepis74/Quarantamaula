#include "ToonObj.h"

ToonObj::ToonObj(const std::string& inModel, const glm::vec3& inPosition, const glm::vec3& inScale, const glm::vec3& inOrientation, const glm::vec3& inBaseColor) : 
GameObject(inPosition, inScale, inOrientation), model(inModel), baseColor(inBaseColor) {}

void ToonObj::draw()
{
	//Get the shader attached to the gameObject
	unsigned int shaderID = ResourceManager::getShader("geometryPass")->getID();

	//Compute the model matrix
	glm::mat4 modelMat = GameObject::computeModelMat();

	//Send model matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, false, glm::value_ptr(modelMat));

	//Set parameters in the shader
	glUniform3fv(glGetUniformLocation(shaderID, "baseColor"), 1, glm::value_ptr(baseColor));

	//Call the model's draw function. We don't need texture data so we send 0 as shaderID
	ResourceManager::getModel(model)->draw(shaderID);
}

void ToonObj::drawGeometry(unsigned int shaderID)
{
	//Compute the model matrix
	glm::mat4 modelMat = GameObject::computeModelMat();

	//Send model matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, false, glm::value_ptr(modelMat));

	//Call the model's draw function. We don't need texture data so we send 0 as shaderID
	ResourceManager::getModel(model)->draw(0);
}
