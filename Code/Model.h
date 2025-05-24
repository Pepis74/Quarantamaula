#ifndef MODEL_H
#define MODEL_H

#include <map>
#include "Mesh.h"

extern "C" {
#include "Libraries\mikktspace.h"
}

struct UserData {
	std::vector<Vertex>* verts;
	std::vector<unsigned int>* idx;
};

class Model {

public:
	//Default constructor
	Model() {}
	//
	Model(const char* filePath);
	//
	void draw(unsigned int shaderID);

private:

	void computeMikkTangentSpace(std::vector<Vertex>& verts, const std::vector<unsigned int>& idx);

	//The model's meshes
	std::vector<Mesh> meshes;
	//
	std::string directory;
	//Model's list of textures with an associated name the meshes can reference
	std::map<std::string, Texture> textures;
};

#endif
