#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include <glad/glad.h>
#include "Libraries/stb_image.h"

class Texture
{
public:
    //Default constructor
    Texture() {}
    //@Params: image's filepath. Creates and configurates texture object and attaches image to texture object 
    Texture(const char* filePath, const char* inType);
    //Returns the texture's ID
    unsigned int getID();
    //Returns the texture's type
    const std::string& getType();

private:
    //ID of the texture object
    unsigned int iD = 0;
    //Diffuse or specular
    std::string type;
};

#endif