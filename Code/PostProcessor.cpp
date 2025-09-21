#include "PostProcessor.h"

void PostProcessor::init(unsigned int* inGBufferTextures)
{
	gBufferTextures = inGBufferTextures;

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

}

void PostProcessor::ambientOcclusion()
{
	
}
