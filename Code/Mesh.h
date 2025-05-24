#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"
#include "Texture.h"
#include "QUtils.h"

#define MAX_TEXTURES 4
#define MAX_POINT_LIGHTS 5

struct Vertex 
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	// w is MikkTSpace handedness
	glm::vec4 tangent;
};

class Mesh {

public:
	//
	Mesh() {};
	//
	Mesh(const std::vector<Vertex>& inVertices, const std::vector<unsigned int>& inIndices, Texture* inDiffTex, Texture* inSpecTex,
		Texture* inNormalMap, Texture* inRoughnessTex, Texture* inMetallicTex, Texture* inClearcoatTex);
	//
	void draw(unsigned int shaderID);
	//
	const std::vector<Vertex>& getVertices();
	//
	const std::vector<unsigned int>& getIndices();

private:
	//
	std::vector<Vertex> vertices;
	//
	std::vector<unsigned int> indices;
	//
	Texture* diffuseTex;
	//
	Texture* specularTex;
	//
	Texture* normalMap;
	//
	Texture* roughnessTex;
	//
	Texture* metallicTex;
	//
	Texture* clearcoatTex;
	//
	unsigned int vAO = 0;
};

#endif
