#include "ShadowFiltersTexture.h"

ShadowFiltersTexture::ShadowFiltersTexture(int mapSize, int filterSize)
{
    //Vector that stores the coordinates of the sample points that compose the randomly generated filters
    std::vector<float> data;
    int bufferSize = mapSize * mapSize * filterSize * filterSize * 2;
    data.resize(bufferSize);

    //Random generator and distribution to generate the random filters
    static std::default_random_engine generator;
    static std::uniform_real_distribution<float> distrib(-0.5f, 0.5f);

    int vectorIndex = 0;

    //Generate the filters
    for (int windowX = 0; windowX < mapSize; windowX++) {
        for (int windowY = 0; windowY < mapSize; windowY++) {
            //We go backwards to have the sample points in the outer circle first
            for (int filterV = filterSize - 1; filterV >= 0; filterV--) {
                for (int filterU = 0; filterU < filterSize; filterU++) {
                    //Each sample point starts located at the center of their grid cell
                    float sampleX = filterU + 0.5;
                    float sampleY = filterV + 0.5;

                    //Then we apply a random jitter to the sample point
                    sampleX += distrib(generator);
                    sampleY += distrib(generator);

                    //We normalize the result
                    sampleX /= filterSize;
                    sampleY /= filterSize;

                    //Finally we warp it to form a circular filter
                    data[vectorIndex] = glm::sqrt(sampleY) * glm::cos(2 * glm::pi<float>() * sampleX);
                    data[vectorIndex + 1] = glm::sqrt(sampleY) * glm::sin(2 * glm::pi<float>() * sampleX);

                    vectorIndex += 2;
                }
            }
        }
    }

    int numSamples = filterSize * filterSize;

    //We generate the 3D texture we're gonna store the filter data in
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &iD);
    glBindTexture(GL_TEXTURE_3D, iD);
    //Allocate space for the filter data. We need only half the numSamples for the width since each texel is going to store two samples
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, numSamples / 2, mapSize, mapSize, 0, GL_RGBA, GL_FLOAT, NULL);
    //Upload the filter data
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,  numSamples / 2, mapSize, mapSize, GL_RGBA, GL_FLOAT, &data[0]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_3D, 0);
}

unsigned int ShadowFiltersTexture::getID()
{
    return iD;
}