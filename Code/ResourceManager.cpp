#include"ResourceManager.h"

//Instantiate static variables
std::map<std::string, Model> ResourceManager::models;
std::map<std::string, Shader> ResourceManager::shaders;
std::map<std::string, unsigned int> ResourceManager::envTextures;


//@Params : vertex shader filepath, fragment shader filepath, shader name for the map. Reads the shader source files, constructs a shader with that source code and then adds it to the
//shaders map
Shader* const ResourceManager::loadShader(const char* vertexPath, const char* fragmentPath, const std::string& name)
{
    //1.Retrieve the vertex/fragment source code from filePath
    std::string tempVertexCode, tempFragmentCode;
    std::ifstream vertexFile, fragmentFile;

    //Ensure ifstream objects can throw exceptions:
    vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragmentFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        //Open files
        vertexFile.open(vertexPath);
        fragmentFile.open(fragmentPath);
        std::stringstream vertexStream, fragmentStream;

        //Read file's buffer contents into streams
        vertexStream << vertexFile.rdbuf();
        fragmentStream << fragmentFile.rdbuf();

        //Close file handlers
        vertexFile.close();
        fragmentFile.close();

        //Convert stream into string
        tempVertexCode = vertexStream.str();
        tempFragmentCode = fragmentStream.str();
    }

    catch (std::ifstream::failure e)
    {
        std::cerr << "Failed to read shader source code.\n";
    }

    const char* vertexCode = tempVertexCode.c_str();
    const char* fragmentCode = tempFragmentCode.c_str();

    //2.Create the shader object and add it to the map
    shaders.emplace(name, Shader(vertexCode, fragmentCode));
    return &shaders[name];
}

//@Params : vertex shader filepath, geometry shader filepath, fragment shader filepath, shader name for the map. Reads the shader source files, constructs a shader with that source code and then adds
//it to the shaders map
Shader* const ResourceManager::loadShader(const char* vertexPath, const char* geometryPath, const char* fragmentPath, const std::string& name)
{
    //1.Retrieve the vertex/fragment source code from filePath
    std::string tempVertexCode, tempGeometryCode, tempFragmentCode;
    std::ifstream vertexFile, geometryFile, fragmentFile;

    //Ensure ifstream objects can throw exceptions:
    vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    geometryFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragmentFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        //Open files
        vertexFile.open(vertexPath);
        geometryFile.open(geometryPath);
        fragmentFile.open(fragmentPath);
        std::stringstream vertexStream, geometryStream, fragmentStream;

        //Read file's buffer contents into streams
        vertexStream << vertexFile.rdbuf();
        geometryStream << geometryFile.rdbuf();
        fragmentStream << fragmentFile.rdbuf();

        //Close file handlers
        vertexFile.close();
        geometryFile.close();
        fragmentFile.close();

        //Convert stream into string
        tempVertexCode = vertexStream.str();
        tempGeometryCode = geometryStream.str();
        tempFragmentCode = fragmentStream.str();
    }

    catch (std::ifstream::failure e)
    {
        std::cerr << "Failed to read shader source code.\n";
    }

    const char* vertexCode = tempVertexCode.c_str();
    const char* geometryCode = tempGeometryCode.c_str();
    const char* fragmentCode = tempFragmentCode.c_str();

    //2.Create the shader object and add it to the map
    shaders.emplace(name, Shader(vertexCode, geometryCode, fragmentCode));
    return &shaders[name];
}

Model* const ResourceManager::loadModel(const char* filePath, const std::string& name)
{
    models.emplace(name, Model(filePath));
    return &models[name];
}

unsigned int ResourceManager::loadEnvTexture(const char* filePath, const std::string& name, bool flipVertically)
{
    if (!flipVertically) stbi_set_flip_vertically_on_load(false);

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrComponents;
    float* data = stbi_loadf(filePath, &width, &height, &nrComponents, 0);

    switch (nrComponents) {
    case 1:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_FLOAT, data);
        break;
    case 2:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, data);
        break;
    case 3:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
        break;
    case 4:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);
        break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(true);

    envTextures.emplace(name, textureID);
    return textureID;
}

unsigned int ResourceManager::loadEnvCubemap(const char* filePath, const std::string& name, bool flipVertically)
{
    if (!flipVertically) stbi_set_flip_vertically_on_load(false);;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    std::string faceNames[] = { "px.hdr", "nx.hdr", "py.hdr", "ny.hdr", "pz.hdr", "nz.hdr" };

    int width, height, nrComponents;
    for (unsigned int i = 0; i < 6; i++)
    {
        float* data = stbi_loadf((filePath + std::string("/") + faceNames[i]).c_str(), &width, &height, &nrComponents, 0);
        
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faceNames[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(true);

    envTextures.emplace(name, textureID);
    return textureID;
}

//@Params : name of the shader we want to retrieve. Returns a specific shader from the loaded shaders
Shader* const ResourceManager::getShader(const std::string& name)
{
    return &shaders[name];
}

Model* const ResourceManager::getModel(const std::string& name)
{
    return &models[name];
}

unsigned int ResourceManager::getEnvTexture(const std::string& name)
{
    return envTextures[name];
}

//Deletes all the stored data
void ResourceManager::clear()
{
    for (auto iter : shaders) 
    {
        glDeleteProgram(iter.second.getID());
    }

    //Delete models too lazy to do it rn 
}
