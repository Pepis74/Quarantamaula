#include "Model.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

std::string& Ltrim(std::string& str)
{
    auto it = std::find_if(str.begin(), str.end(), [](char ch) { return (!std::isspace(ch)); });
    str.erase(str.begin(), it);
    return (str);
}

std::string& Rtrim(std::string& str)
{
    auto it = std::find_if(str.rbegin(), str.rend(), [](char ch) { return (!std::isspace(ch)); });
    str.erase(it.base(), str.end());
    return (str);
}

std::string& Trim(std::string& str)
{
    return (Ltrim(Rtrim(str)));
}

std::vector<std::string> Split(const std::string& str, char splitChar, bool ignoreEmpty = true) {
    std::stringstream origStr(str);
    std::string segment;
    std::vector<std::string> seglist;

    while (std::getline(origStr, segment, splitChar))
    {
        if (!(segment.empty() && ignoreEmpty)) seglist.push_back(segment);
    }
    return seglist;
}

const char kPathSeparator = '/';

std::pair<std::string, std::string> SplitFilename(const std::string& str)
{
    size_t found;
    found = str.find_last_of("/\\");
    return { str.substr(0, found), str.substr(found + 1) };
}

//----------------------------------------------------------------------
// Model::Model
//
// Loads an OBJ file directly (without Assimp) and uses an internal MTL
// parser to load material textures. When a "usemtl" line is found, it looks
// up whether the material has a map_Kd, map_Ks and/or map_bump, adds them
// to the textures map, and passes the texture pointers to the Mesh constructor.
//----------------------------------------------------------------------
Model::Model(const char* filePath)
{
    // Open the OBJ file.
    std::ifstream objFile(filePath);
    if (!objFile.is_open())
    {
        std::cerr << "ERROR: Could not open OBJ file: " << filePath << std::endl;
        return;
    }

    // Save directory (used for loading textures/MTL files).
    std::string sPath(filePath);
    directory = sPath.substr(0, sPath.find_last_of("/\\"));

    // Global arrays to store vertex data.
    std::vector<glm::vec3> globalVertices;
    std::vector<glm::vec3> globalNormals;
    std::vector<glm::vec2> globalTexCoords;

    // Buffers for the current mesh.
    std::vector<Vertex> currVertices;
    std::vector<unsigned int> currIndices;

    // Pointers to textures (for diffuse, specular, normal maps).
    Texture* diffuseTex = nullptr;
    Texture* specularTex = nullptr;
    Texture* normalMap = nullptr;
    Texture* roughnessTex = nullptr;
    Texture* metallicTex = nullptr;
    Texture* clearcoatTex = nullptr;

    // Track the current material name.
    std::string currentMaterialName = "";
    // Default: if no material is set, textures remain nullptr.

    std::string line;
    while (std::getline(objFile, line))
    {
        line = Trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        std::vector<std::string> tokens = Split(line, ' ');
        if (tokens.empty())
            continue;

        std::string token = tokens[0];

        if (token == "mtllib")
        {
            // When an MTL file is referenced, build the full path and parse it.
            if (tokens.size() >= 2)
            {
                std::string mtlFilePath = directory + kPathSeparator + Trim(tokens[1]);
                // Parse the MTL file to load material textures.
                // This function will fill the textures map.
                std::ifstream mtlFile(mtlFilePath);
                if (!mtlFile.is_open())
                {
                    std::cerr << "ERROR: Could not open MTL file: " << mtlFilePath << std::endl;
                }
                else
                {
                    std::string mtlLine;
                    std::string currentMat;
                    while (std::getline(mtlFile, mtlLine))
                    {
                        mtlLine = Trim(mtlLine);
                        if (mtlLine.empty() || mtlLine[0] == '#')
                            continue;
                        auto mtlTokens = Split(mtlLine, ' ');
                        if (mtlTokens.empty())
                            continue;
                        if (mtlTokens[0] == "newmtl")
                        {
                            if (mtlTokens.size() >= 2)
                                currentMat = mtlTokens[1];
                        }
                        else if (mtlTokens[0] == "map_Kd")
                        {
                            if (mtlTokens.size() >= 2 && !currentMat.empty())
                            {
                                std::string texPath = directory + kPathSeparator + Trim(mtlTokens[1]);
                                // Store in textures map with key: material_diffuse

                                textures[currentMat + "_diffuse"] = Texture(texPath.c_str(), "diffuse");
                            }
                        }
                        else if (mtlTokens[0] == "map_Ks")
                        {
                            if (mtlTokens.size() >= 2 && !currentMat.empty())
                            {
                                std::string texPath = directory + kPathSeparator + Trim(mtlTokens[1]);
                                textures[currentMat + "_specular"] = Texture(texPath.c_str(), "specular");
                            }
                        }
                        else if (mtlTokens[0] == "map_Bump" || mtlTokens[0] == "bump")
                        {
                            if (mtlTokens.size() >= 2 && !currentMat.empty())
                            {
                                std::string texPath = directory + kPathSeparator + Trim(mtlTokens[1]);
                                textures[currentMat + "_bump"] = Texture(texPath.c_str(), "normal");
                            }
                        }
                        else if (mtlTokens[0] == "map_Roughness")
                        {
                            if (mtlTokens.size() >= 2 && !currentMat.empty())
                            {
                                std::string texPath = directory + kPathSeparator + Trim(mtlTokens[1]);
                                textures[currentMat + "_roughness"] = Texture(texPath.c_str(), "roughness");
                            }
                        }
                        else if (mtlTokens[0] == "map_Metallic")
                        {
                            if (mtlTokens.size() >= 2 && !currentMat.empty())
                            {
                                std::string texPath = directory + kPathSeparator + Trim(mtlTokens[1]);
                                textures[currentMat + "_metallic"] = Texture(texPath.c_str(), "metallic");
                            }
                        }
                        else if (mtlTokens[0] == "map_Clearcoat")
                        {
                            if (mtlTokens.size() >= 2 && !currentMat.empty())
                            {
                                std::string texPath = directory + kPathSeparator + Trim(mtlTokens[1]);
                                textures[currentMat + "_clearcoat"] = Texture(texPath.c_str(), "clearcoat");
                            }
                        }
                    }
                }
            }
        }
        else if (token == "v")
        {
            // Vertex position.
            if (tokens.size() < 4)
                continue;
            float x = std::stof(tokens[1]);
            float y = std::stof(tokens[2]);
            float z = std::stof(tokens[3]);
            globalVertices.push_back(glm::vec3(x, y, z));
        }
        else if (token == "vn")
        {
            // Vertex normal.
            if (tokens.size() < 4)
                continue;
            float x = std::stof(tokens[1]);
            float y = std::stof(tokens[2]);
            float z = std::stof(tokens[3]);
            glm::vec3 normal(x, y, z);
            normal = glm::normalize(normal);
            globalNormals.push_back(normal);
        }
        else if (token == "vt")
        {
            // Texture coordinate.
            if (tokens.size() < 3)
                continue;
            float u = std::stof(tokens[1]);
            float v = std::stof(tokens[2]);
            globalTexCoords.push_back(glm::vec2(u, v));
        }
        else if (token == "f")
        {
            // Face (assumed triangular).
            if (tokens.size() < 4)
                continue;
            for (int i = 1; i <= 3; i++)
            {
                std::vector<std::string> subTokens = Split(tokens[i], '/', false);
                int vIndex = (subTokens.size() > 0 && !subTokens[0].empty()) ? std::stoi(subTokens[0]) - 1 : -1;
                int vtIndex = (subTokens.size() > 1 && !subTokens[1].empty()) ? std::stoi(subTokens[1]) - 1 : -1;
                int vnIndex = (subTokens.size() > 2 && !subTokens[2].empty()) ? std::stoi(subTokens[2]) - 1 : -1;

                Vertex vert;
                if (vIndex >= 0 && vIndex < (int)globalVertices.size())
                    vert.position = globalVertices[vIndex];
                else
                    vert.position = glm::vec3(0.f, 0.f, 0.f);

                if (vtIndex >= 0 && vtIndex < (int)globalTexCoords.size())
                    vert.texCoords = globalTexCoords[vtIndex];
                else
                    vert.texCoords = glm::vec2(0.f, 0.f);

                if (vnIndex >= 0 && vnIndex < (int)globalNormals.size())
                    vert.normal = globalNormals[vnIndex];
                else
                    vert.normal = glm::vec3(0.f, 1.f, 0.f);

                currVertices.push_back(vert);
                currIndices.push_back(currVertices.size() - 1);
            }
        }
        else if (token == "o" || token == "g")
        {
            // When a new object or group is encountered, if there is existing data,
            // create a new Mesh from the accumulated vertices and indices.
            if (!currVertices.empty())
            {
                computeMikkTangentSpace(currVertices, currIndices);
                meshes.push_back(Mesh(currVertices, currIndices, diffuseTex, specularTex, normalMap, roughnessTex, metallicTex, clearcoatTex));
                currVertices.clear();
                currIndices.clear();
            }
            // Optionally, you can update the current group name here.
        }
        else if (token == "usemtl")
        {
            // When a new material is used, update the texture pointers.
            if (tokens.size() >= 2)
            {
                std::string matName = tokens[1];
                currentMaterialName = matName;
                // Look up textures in the textures map.
                if (textures.find(matName + "_diffuse") != textures.end())
                    diffuseTex = &textures[matName + "_diffuse"];
                else
                    diffuseTex = nullptr;
                if (textures.find(matName + "_specular") != textures.end())
                    specularTex = &textures[matName + "_specular"];
                else
                    specularTex = nullptr;
                if (textures.find(matName + "_bump") != textures.end())
                    normalMap = &textures[matName + "_bump"];
                else
                    normalMap = nullptr;
                if (textures.find(matName + "_roughness") != textures.end())
                    roughnessTex = &textures[matName + "_roughness"];
                else
                    roughnessTex = nullptr;
                if (textures.find(matName + "_metallic") != textures.end())
                    metallicTex = &textures[matName + "_metallic"];
                else
                    metallicTex = nullptr;
                if (textures.find(matName + "_clearcoat") != textures.end())
                    clearcoatTex = &textures[matName + "_clearcoat"];
                else
                    clearcoatTex = nullptr;
            }
        }
    }
    // Finalize the last mesh.
    if (!currVertices.empty())
    {
        // Compute MikkTSpace tangents before handing vertices to the mesh
        computeMikkTangentSpace(currVertices, currIndices);
        meshes.push_back(Mesh(currVertices, currIndices, diffuseTex, specularTex, normalMap, roughnessTex, metallicTex, clearcoatTex));
        currVertices.clear();
        currIndices.clear();
        std::cout << filePath << " model created" << std::endl;
    }
}

//----------------------------------------------------------------------
// Model::draw
//
// Iterates through each mesh and calls its draw() method.
//----------------------------------------------------------------------
void Model::draw(unsigned int shaderID)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].draw(shaderID);
    }
}

// Callback: how many faces?
static int getNumFaces(const SMikkTSpaceContext* ctx) {
    auto* ud = (UserData*)ctx->m_pUserData;
    return int(ud->idx->size() / 3);
}

// Callback: 3 verts per face (triangles)
static int getNumVerticesOfFace(const SMikkTSpaceContext*, int) {
    return 3;
}

// Callback: retrieve position of one corner
static void getPosition(const SMikkTSpaceContext* ctx,
    float fvPosOut[], const int face, const int vert) {
    auto* ud = (UserData*)ctx->m_pUserData;
    unsigned int meshIndex = (*ud->idx)[face * 3 + vert];
    const auto& p = (*ud->verts)[meshIndex].position;
    fvPosOut[0] = p.x; fvPosOut[1] = p.y; fvPosOut[2] = p.z;
}

// Callback: retrieve normal
static void getNormal(const SMikkTSpaceContext* ctx,
    float fvNormOut[], const int face, const int vert) {
    auto* ud = (UserData*)ctx->m_pUserData;
    unsigned int meshIndex = (*ud->idx)[face * 3 + vert];
    const auto& n = (*ud->verts)[meshIndex].normal;
    fvNormOut[0] = n.x; fvNormOut[1] = n.y; fvNormOut[2] = n.z;
}

// Callback: retrieve UV
static void getTexCoord(const SMikkTSpaceContext* ctx,
    float fvTexcOut[], const int face, const int vert) {
    auto* ud = (UserData*)ctx->m_pUserData;
    unsigned int meshIndex = (*ud->idx)[face * 3 + vert];
    const auto& uv = (*ud->verts)[meshIndex].texCoords;
    fvTexcOut[0] = uv.x; fvTexcOut[1] = uv.y;
}

// Callback: store the computed tangent + handedness
static void setTSpaceBasic(const SMikkTSpaceContext* ctx,
    const float tangent[3], const float sign,
    const int face, const int vert) {
    auto* ud = (UserData*)ctx->m_pUserData;
    unsigned int meshIndex = (*ud->idx)[face * 3 + vert];
    (*ud->verts)[meshIndex].tangent =
        glm::vec4(tangent[0], tangent[1], tangent[2], sign);
}

void Model::computeMikkTangentSpace(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
    // Fill out the MikkTSpace interface
    SMikkTSpaceInterface mikkInterface = {};
    mikkInterface.m_getNumFaces = getNumFaces;
    mikkInterface.m_getNumVerticesOfFace = getNumVerticesOfFace;
    mikkInterface.m_getPosition = getPosition;
    mikkInterface.m_getNormal = getNormal;
    mikkInterface.m_getTexCoord = getTexCoord;
    mikkInterface.m_setTSpaceBasic = setTSpaceBasic;

    // Pack your arrays into one struct
    UserData userData{ &vertices, (std::vector<unsigned int>*) & indices };

    // Create and run the MikkTSpace context
    SMikkTSpaceContext mikkContext;
    mikkContext.m_pInterface = &mikkInterface;
    mikkContext.m_pUserData = &userData;

    genTangSpaceDefault(&mikkContext);
}

