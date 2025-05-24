#include "BlinnPhongObj.h"

BlinnPhongObj::BlinnPhongObj(const std::string& inModel, const glm::vec3& inPosition, const glm::vec3& inScale, const glm::vec3& inOrientation,
	const glm::vec3& inBaseColor, float inSpecular) : GameObject(inPosition, inScale, inOrientation),
	model(inModel), baseColor(inBaseColor), specular(inSpecular) {}

void BlinnPhongObj::draw()
{
	//Get the shader attached to the gameObject
	unsigned int shaderID = ResourceManager::getShader("geometryPass")->getID();

	//Compute the model matrix
	glm::mat4 modelMat = GameObject::computeModelMat();

	//Send model matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, false, glm::value_ptr(modelMat));

	//Compute the normal matrix and send it to the shader
	glm::mat3 normalMat = glm::transpose(glm::inverse(modelMat));
	glUniformMatrix3fv(glGetUniformLocation(shaderID, "normalMat"), 1, false, glm::value_ptr(normalMat));

	//Set parameters in the shader
	glUniform3fv(glGetUniformLocation(shaderID, "baseColor"), 1, glm::value_ptr(baseColor));
	glUniform1f(glGetUniformLocation(shaderID, "specular"), specular);

	//Call the model's draw function
	ResourceManager::getModel(model)->draw(shaderID);
}

void BlinnPhongObj::drawGeometry(unsigned int shaderID)
{
	//Compute the model matrix
	glm::mat4 modelMat = GameObject::computeModelMat();

	//Send model matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, false, glm::value_ptr(modelMat));

	//Call the model's draw function. We don't need texture data so we send 0 as shaderID
	ResourceManager::getModel(model)->draw(0);
}
