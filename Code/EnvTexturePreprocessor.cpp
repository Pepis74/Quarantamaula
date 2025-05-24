#include "EnvTexturePreprocessor.h"

unsigned int EnvTexturePreprocessor::screenQuadVAO;
unsigned int EnvTexturePreprocessor::cubeVAO;
unsigned int EnvTexturePreprocessor::cubeNVertices;
unsigned int EnvTexturePreprocessor::fBO;
unsigned int EnvTexturePreprocessor::brdfFBO;
unsigned int EnvTexturePreprocessor::depthRBO;
unsigned int EnvTexturePreprocessor::irradianceMap;
unsigned int EnvTexturePreprocessor::specularMap;
unsigned int EnvTexturePreprocessor::brdfIntegrationMap;

void EnvTexturePreprocessor::init(unsigned int inScreenQuadVAO) 
{
    screenQuadVAO = inScreenQuadVAO;

    float vertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    //Generate the VAO
    glGenVertexArrays(1, &cubeVAO);

    //Declare variables where we will store the VBO's and EBO's IDs
    unsigned int vBO;

    //Generate VBO
    glGenBuffers(1, &vBO);

    //Bind VAO we want to use to store our VBO, attribpointers and EBO
    glBindVertexArray(cubeVAO);

    //Pass our vertex data to the GPU using the VBO
    glBindBuffer(GL_ARRAY_BUFFER, vBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Set up the vertex attribute pointers which will tell OpenGL how to interpret the data in each vertex. 
    glEnableVertexAttribArray(0);

    //Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    //Unbind the vertex array and buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    cubeNVertices = sizeof(vertices) / (sizeof(float) * 3);

    glGenFramebuffers(1, &fBO);
    glGenFramebuffers(1, &brdfFBO);
    glGenRenderbuffers(1, &depthRBO);
}

void EnvTexturePreprocessor::generateIrradianceMap(const std::string& envTexture)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fBO);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, IRRADIANCE_MAP_RES, IRRADIANCE_MAP_RES);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

	//Allocate memory for each of the cubemao's faces
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
			IRRADIANCE_MAP_RES, IRRADIANCE_MAP_RES, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Projection and view matrices
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

    // Use correct program
    unsigned int shaderID = ResourceManager::getShader("irradianceMap")->getID();
    glUseProgram(shaderID);

    // Set up the texture
    unsigned int envTexID = ResourceManager::getEnvTexture(envTexture);
    QUtils::bindTextureCubemap(shaderID, "envMap", 0, envTexID);

    //Set up projection matrix
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, false, glm::value_ptr(captureProjection));

    // Render each face of the irradiance cubemap
    glViewport(0, 0, IRRADIANCE_MAP_RES, IRRADIANCE_MAP_RES); 
    glBindFramebuffer(GL_FRAMEBUFFER, fBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        // Set up the view matrix for this face of the cube
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, false, glm::value_ptr(captureViews[i]));
        // Attach the corresponding face of the cube to the target framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Render cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, cubeNVertices);
        glBindVertexArray(0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void EnvTexturePreprocessor::generateSpecularMap(const std::string& envTexture)
{
    // Allocate space for the specular map
    glGenTextures(1, &specularMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, specularMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, SPECULAR_MAP_RES, SPECULAR_MAP_RES, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Projection and view matrices
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // Use correct program
    unsigned int shaderID = ResourceManager::getShader("specularMap")->getID();
    glUseProgram(shaderID);

    // Set up the texture
    unsigned int envTexID = ResourceManager::getEnvTexture(envTexture);
    QUtils::bindTextureCubemap(shaderID, "envMap", 0, envTexID);
    // To reduce higher roughness artifacts
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //Set up projection matrix
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, false, glm::value_ptr(captureProjection));

    glBindFramebuffer(GL_FRAMEBUFFER, fBO);
    for (unsigned int mip = 0; mip < ROUGHNESS_MIP_LEVELS; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        // Compute roughness from mip levels and send it to the shader
        float roughness = (float)mip / (float)(ROUGHNESS_MIP_LEVELS - 1);
        glUniform1f(glGetUniformLocation(shaderID, "roughness"), roughness);

        for (unsigned int i = 0; i < 6; ++i)
        {
            // Set up the view matrix for this face of the cube
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, false, glm::value_ptr(captureViews[i]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, specularMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            //Render cube
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, cubeNVertices);
            glBindVertexArray(0);
        }
    }
}

void EnvTexturePreprocessor::generateBRDFIntegrationMap()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glGenTextures(1, &brdfIntegrationMap);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfIntegrationMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, BRDF_MAP_RES, BRDF_MAP_RES, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, brdfFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, BRDF_MAP_RES, BRDF_MAP_RES);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfIntegrationMap, 0);

    glViewport(0, 0, BRDF_MAP_RES, BRDF_MAP_RES);

    unsigned int shaderID = ResourceManager::getShader("brdfIntegrator")->getID();
    glUseProgram(shaderID);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Render quad
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

void EnvTexturePreprocessor::writeIrradianceMap(const std::string& folderPath) {
    // Add error checking for texture binding
    if (glIsTexture(irradianceMap) == GL_FALSE) {
        throw std::runtime_error("Irradiance map not initialized");
    }

    const int resolution = IRRADIANCE_MAP_RES;
    std::vector<float> pixels(resolution * resolution * 3);

    // Create directory if it doesn't exist
    if (!std::filesystem::exists(folderPath)) {
        std::filesystem::create_directories(folderPath);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

    // Face order: +X, -X, +Y, -Y, +Z, -Z
    const std::vector<std::string> faceNames = {
        "px.hdr",  // +X
        "nx.hdr",   // -X
        "py.hdr",   // +Y
        "ny.hdr",   // -Y
        "pz.hdr",   // +Z
        "nz.hdr"    // -Z
    };

    for (int i = 0; i < 6; i++) {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, GL_FLOAT, pixels.data());

        std::string fullPath = (std::filesystem::path(folderPath) / faceNames[i]).string();
        stbi_write_hdr(fullPath.c_str(), resolution, resolution, 3, pixels.data());
    }
}

void EnvTexturePreprocessor::writeBRDFIntegrationMap(const std::string& folderPath) {
    if (glIsTexture(brdfIntegrationMap) == GL_FALSE) {
        throw std::runtime_error("BRDF map not initialized");
    }

    const int resolution = BRDF_MAP_RES;
    std::vector<float> pixels(resolution * resolution * 2);

    // Create directory if it doesn't exist
    if (!std::filesystem::exists(folderPath)) {
        std::filesystem::create_directories(folderPath);
    }

    glBindTexture(GL_TEXTURE_2D, brdfIntegrationMap);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, pixels.data());

    // Pad to 3 channels
    std::vector<float> paddedPixels(resolution * resolution * 3);
    for (size_t i = 0; i < pixels.size(); i += 2) {
        paddedPixels[(i / 2) * 3] = pixels[i];     // R
        paddedPixels[(i / 2) * 3 + 1] = pixels[i + 1]; // G
        paddedPixels[(i / 2) * 3 + 2] = 0.0f;       // B
    }

    std::string fullPath = (std::filesystem::path(folderPath) / "BRDFIntegrationMap.hdr").string();
    stbi_write_hdr(fullPath.c_str(), resolution, resolution, 3, paddedPixels.data());
}
unsigned int EnvTexturePreprocessor::getIrradianceMap()
{
    return irradianceMap;
}

unsigned int EnvTexturePreprocessor::getSpecularMap()
{
    return specularMap;
}

unsigned int EnvTexturePreprocessor::getBrdfIntegrationMap()
{
    return brdfIntegrationMap;
}

unsigned int EnvTexturePreprocessor::getRoughnessMipLevels()
{
    return ROUGHNESS_MIP_LEVELS;
}
