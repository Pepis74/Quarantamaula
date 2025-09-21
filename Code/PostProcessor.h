#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#define SSAO_KERNEL_SAMPLES 64
#define SSAO_TEXTURE_SIZE 4

#include <memory>
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "QUtils.h"

class PostProcessor {
public:

	static void init(unsigned int* gBufferTextures);
	// Perform screen space ambient occlusion, result is stored in tangent texture
	static void ambientOcclusion();

private:
	// Private constructor, no instancing
	PostProcessor();

	static unsigned int* gBufferTextures;
};

#endif
