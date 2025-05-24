#include "Skybox.h"

Skybox::Skybox(const std::string inTexture, const glm::vec3& inPosition, const glm::vec3& inScale, const glm::vec3& inOrientation) : GameObject(inPosition, inScale, inOrientation), 
texture(inTexture)
{
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
    glGenVertexArrays(1, &vAO);

    //Declare variables where we will store the VBO's and EBO's IDs
    unsigned int vBO;

    //Generate VBO
    glGenBuffers(1, &vBO);

    //Bind VAO we want to use to store our VBO, attribpointers and EBO
    glBindVertexArray(vAO);

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

    nVertices = sizeof(vertices) / (sizeof(float) * 3);
}

void Skybox::draw()
{
    unsigned int shaderID = ResourceManager::getShader("skyboxGeometry")->getID();

    //Compute model matrix
    glm::mat4 modelMat = GameObject::computeModelMat();

    //Send model matrix to the shader
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, false, glm::value_ptr(modelMat));

    //Bind skybox texture
    unsigned int textureID = ResourceManager::getEnvTexture("hiRes");
    QUtils::bindTextureCubemap(shaderID, "skybox", 0, textureID);

    //Draw gizmo
    glBindVertexArray(vAO);
    glDrawArrays(GL_TRIANGLES, 0, nVertices);
    glBindVertexArray(0);
}

void Skybox::drawGeometry(unsigned int shaderID)
{

}
