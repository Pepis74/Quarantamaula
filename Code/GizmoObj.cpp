#include "GizmoObj.h"

GizmoObj::GizmoObj(const glm::vec3& inPosition, const glm::vec3& inScale, const glm::vec3& inOrientation, 
	const std::vector<glm::vec3>& inVertices, const glm::vec3& inColor) : GameObject(inPosition, inScale, inOrientation), 
	vertices(inVertices), color(inColor)
{
	//Generate the VAO
	glGenVertexArrays(1, &vAO);

	//Declare variables where we will store the VBO's and EBO's IDs
	unsigned int vBO;

	//Generate VBO
	glGenBuffers(1, &vBO);

	//Bind VAO we want to use to store our VBO, attribpointers and EBO
	glBindVertexArray(vAO);

	//Pass our vertex data to the GPU using the VBO
	glBindBuffer(GL_ARRAY_BUFFER, vBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	//Set up the vertex attribute pointers which will tell OpenGL how to interpret the data in each vertex. 
	glEnableVertexAttribArray(0);

	//Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//Unbind the vertex array and buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GizmoObj::draw()
{
	unsigned int shaderID = ResourceManager::getShader("gizmo")->getID();

	//Compute model matrix
	glm::mat4 modelMat = GameObject::computeModelMat();

	//Send model matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, false, glm::value_ptr(modelMat));

	//Send gizmo's color to the shader
	glUniform3fv(glGetUniformLocation(shaderID, "color"), 1, glm::value_ptr(color));

	//Draw gizmo
	glBindVertexArray(vAO);
	glDrawArrays(GL_LINES, 0, vertices.size());
	glBindVertexArray(0);
}

void GizmoObj::drawGeometry(unsigned int shaderID)
{
	//Compute model matrix
	glm::mat4 modelMat = GameObject::computeModelMat();

	//Send model matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, false, glm::value_ptr(modelMat));

	//Draw gizmo
	glBindVertexArray(vAO);
	glDrawArrays(GL_LINES, 0, vertices.size());
	glBindVertexArray(0);
}

void GizmoObj::setScale(const glm::vec3& inScale)
{
	scale = inScale;
}
