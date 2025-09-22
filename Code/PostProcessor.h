#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#define SSAO_KERNEL_SAMPLES 64
#define SSAO_TEXTURE_SIZE 16
#define SSAO_KERNEL_RADIUS 0.5f
#define SSAO_BIAS 0.025f

#include <memory>
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "QUtils.h"
#include "ResourceManager.h"

class PostProcessor {
public:

	static void init(unsigned int windowWidth, unsigned int windowHeight, unsigned int inScreenQuadVAO, unsigned int* gBufferTextures);
	// Perform screen space ambient occlusion, result is stored in tangent texture
	static void ambientOcclusion();

private:
	// Private constructor, no instancing
	PostProcessor();

	static unsigned int* gBufferTextures;
	static unsigned int screenQuadVAO;
	static unsigned int ssaoKernelsTex;
	static unsigned int ssaoFBO;
};

#endif
