#ifndef SHADOW_FILTERS_TEXTURE_H
#define SHADOW_FILTERS_TEXTURE_H

#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include "glad/glad.h"
#include <iostream>

class ShadowFiltersTexture {

public:
    ShadowFiltersTexture(int windowSize, int filterSize);

    unsigned int getID();

private:
    unsigned int iD = 0;
};

#endif

