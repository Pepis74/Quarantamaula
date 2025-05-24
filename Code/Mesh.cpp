#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices, Texture* inDiffTex, Texture* inSpecTex,
	Texture* inNormalMap, Texture* inRoughnessTex, Texture* inMetallicTex, Texture* inClearcoatTex) : 
	vertices(inVertices), indices(inIndices), diffuseTex(inDiffTex), specularTex(inSpecTex), normalMap(inNormalMap), roughnessTex(inRoughnessTex), metallicTex(inMetallicTex),
	clearcoatTex(inClearcoatTex)
{
	//Generate the VAO
	glGenVertexArrays(1, &vAO);

	//Declare variables where we will store the VBO's and EBO's IDs
	unsigned int vBO, eBO;

	//Generate VBO and EBO
	glGenBuffers(1, &vBO);
	glGenBuffers(1, &eBO);

	//Bind VAO we want to use to store our VBO, attribpointers and EBO
	glBindVertexArray(vAO);

	//Pass our vertex data to the GPU using the VBO
	glBindBuffer(GL_ARRAY_BUFFER, vBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	//Pass the order in which vertices must be rendered to the GPU using the EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	//Set up the vertex attribute pointers which will tell OpenGL how to interpret the data in each vertex. 
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	//Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	//Normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	//Texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	//Tangents
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

	//Unbind the vertex array and buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::draw(unsigned int shaderID)
{
	//If the shader ID passed is 0 that means we don't need the texture data
	if (shaderID > 0) 
	{
		// Bind the normal map (texture unit 0)
		if (normalMap)
		{
			QUtils::bindTexture2D(shaderID, "normalMap", 0, normalMap->getID());
			glUniform1ui(glGetUniformLocation(shaderID, "hasNormalMap"), 1);
		}
		else
		{
			glUniform1ui(glGetUniformLocation(shaderID, "hasNormalMap"), 0);
		}

		// Bind the diffuse map (texture unit 1)
		if (diffuseTex)
		{
			QUtils::bindTexture2D(shaderID, "diffuseMap", 1, diffuseTex->getID());
			glUniform1ui(glGetUniformLocation(shaderID, "hasDiffuseMap"), 1);
		}
		else
		{
			glUniform1ui(glGetUniformLocation(shaderID, "hasDiffuseMap"), 0);
		}

		// Bind the roughness map (texture unit 2)
		if (roughnessTex)
		{
			QUtils::bindTexture2D(shaderID, "roughnessMap", 2, roughnessTex->getID());
			glUniform1ui(glGetUniformLocation(shaderID, "hasRoughnessMap"), 1);
		}
		else
		{
			glUniform1ui(glGetUniformLocation(shaderID, "hasRoughnessMap"), 0);
		}

		// Bind the specular map (texture unit 3)
		if (specularTex)
		{
			QUtils::bindTexture2D(shaderID, "specularMap", 3, specularTex->getID());
			glUniform1ui(glGetUniformLocation(shaderID, "hasSpecularMap"), 1);
		}
		else
		{
			glUniform1ui(glGetUniformLocation(shaderID, "hasSpecularMap"), 0);
		}

		// Bind the metallic map (texture unit 4)
		if (metallicTex)
		{
			QUtils::bindTexture2D(shaderID, "metallicMap", 4, metallicTex->getID());
			glUniform1ui(glGetUniformLocation(shaderID, "hasMetallicMap"), 1);
		}
		else
		{
			glUniform1ui(glGetUniformLocation(shaderID, "hasMetallicMap"), 0);
		}

		// The anisotropic map (texture unit 5) is not supported; set its flag to 0.
		glUniform1ui(glGetUniformLocation(shaderID, "hasAnisotropicMap"), 0);

		// Bind the clearcoat map (texture unit 6)
		if (clearcoatTex)
		{
			QUtils::bindTexture2D(shaderID, "clearcoatMap", 6, clearcoatTex->getID());
			glUniform1ui(glGetUniformLocation(shaderID, "hasClearcoatMap"), 1);
		}
		else
		{
			glUniform1ui(glGetUniformLocation(shaderID, "hasClearcoatMap"), 0);
		}

		// The clearcoat roughness map (texture unit 7) is not supported; set its flag to 0.
		glUniform1ui(glGetUniformLocation(shaderID, "hasClearcoatRoughnessMap"), 0);
	}
	
	//Draw the mesh
	glBindVertexArray(vAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

const std::vector<Vertex>& Mesh::getVertices()
{
	return vertices;
}

const std::vector<unsigned int>& Mesh::getIndices()
{
	return indices;
}