#include "PostProcessor.h"

unsigned int* PostProcessor::gBufferTextures;
unsigned int PostProcessor::screenQuadVAO;
unsigned int PostProcessor::ssaoKernelsTex;
unsigned int PostProcessor::ssaoFBO;

void PostProcessor::init(unsigned int windowWidth, unsigned int windowHeight, unsigned int inScreenQuadVAO, unsigned int* inGBufferTextures)
{
	gBufferTextures = inGBufferTextures;
    screenQuadVAO = inScreenQuadVAO;

	// Prepare the ssao samples 3D texture
   
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;

    // Data for the kernel samples 3D texture, each texel contains a kernel (collection of samples)
    std::vector<glm::vec3> data;

    for (unsigned int i = 0; i < SSAO_TEXTURE_SIZE * SSAO_TEXTURE_SIZE; i++) {
        // Prepare the kernel, with more sample points closer to the origin
        for (unsigned int j = 0; j < SSAO_KERNEL_SAMPLES; ++j)
        {
            glm::vec3 sample(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator)
            );
            sample = glm::normalize(sample);
            float scale = (float)j / SSAO_KERNEL_SAMPLES;
            scale = QUtils::lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            data.push_back(sample);
        }
    }

    //We generate the 3D texture we're gonna store the kernel data in
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &ssaoKernelsTex);
    glBindTexture(GL_TEXTURE_3D, ssaoKernelsTex);
    //Allocate space for the kernel data. 
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, SSAO_KERNEL_SAMPLES, SSAO_TEXTURE_SIZE, SSAO_TEXTURE_SIZE, 0, GL_RGB, GL_FLOAT, NULL);
    //Upload the filter data
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, SSAO_KERNEL_SAMPLES, SSAO_TEXTURE_SIZE, SSAO_TEXTURE_SIZE, GL_RGB, GL_FLOAT, &data[0]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_3D, 0);

    //Create ssao framebuffer
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    //We bind the G buffer's fifth texture, because the ssao will be output in its alpha channel
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBufferTextures[4], 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::ambientOcclusion()
{
    //Bind the SSAO framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	//We modify the color mask to only write over the alpha channel and not other data in the rgb channels of the texture
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);

    unsigned int shaderID = ResourceManager::getShader("ssao")->getID();
    glUseProgram(shaderID);

    //Set g buffer texture units in the shader
    QUtils::bindTexture2D(shaderID, "gPositionRoughness", 0, gBufferTextures[0]);
    QUtils::bindTexture2D(shaderID, "gNormalsSpecular", 1, gBufferTextures[1]);
    QUtils::bindTexture2D(shaderID, "gTangents", 2, gBufferTextures[2]);
    QUtils::bindTexture3D(shaderID, "kernelsTex", 3, ssaoKernelsTex);

    // Set the SSAO parameters in the shader
    glUniform1ui(glGetUniformLocation(shaderID, "kernelSamples"), SSAO_KERNEL_SAMPLES);
    glUniform1ui(glGetUniformLocation(shaderID, "kernelsTexSize"), SSAO_TEXTURE_SIZE);
    glUniform1f(glGetUniformLocation(shaderID, "kernelRadius"), SSAO_KERNEL_RADIUS);
    glUniform1f(glGetUniformLocation(shaderID, "bias"), SSAO_BIAS);

    //Bind the VAO and make draw the screen quad
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    //Restore regular rendering state
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
