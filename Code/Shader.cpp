#include "Shader.h"

//@Params : vertex shader source code, fragment shader source code. Creates and links the shader program using the given shader source code
Shader::Shader(const char* vertexCode, const char* fragmentCode)
{
    //1.Compile shaders
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    //Create the vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexCode, NULL);
    glCompileShader(vertex);

    //Print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

    if (success == 0)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "Failed to compile vertex shader: \n" << infoLog << '\n';
    }

    //Create the fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentCode, NULL);
    glCompileShader(fragment);

    //Print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

    if (success == 0)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "Failed to compile fragment shader: \n" << infoLog << '\n';
    }

    //2.Create and link the shader program
    iD = glCreateProgram();
    glAttachShader(iD, vertex);
    glAttachShader(iD, fragment);
    glLinkProgram(iD);

    //Print linking errors if any
    glGetProgramiv(iD, GL_LINK_STATUS, &success);

    if (success == 0)
    {
        glGetProgramInfoLog(iD, 512, NULL, infoLog);
        std::cerr << "Failed to link shader program: \n" << infoLog << '\n';
    }

    //3.Delete the shaders 
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

//@Params : vertex shader source code, geometry shader source code, fragment shader source code. Creates and links the shader program using the given shader source code
Shader::Shader(const char* vertexCode, const char* geometryCode, const char* fragmentCode)
{
    //1.Compile shaders
    unsigned int vertex, geometry, fragment;
    int success;
    char infoLog[512];

    //Create the vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexCode, NULL);
    glCompileShader(vertex);

    //Print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

    if (success == 0)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "Failed to compile vertex shader: \n" << infoLog << '\n';
    }

    //Create the geometry shader shader
    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, &geometryCode, NULL);
    glCompileShader(geometry);

    //Print compile errors if any
    glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);

    if (success == 0)
    {
        glGetShaderInfoLog(geometry, 512, NULL, infoLog);
        std::cerr << "Failed to compile geometry shader: \n" << infoLog << '\n';
    }

    //Create the fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentCode, NULL);
    glCompileShader(fragment);

    //Print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

    if (success == 0)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "Failed to compile fragment shader: \n" << infoLog << '\n';
    }

    //2.Create and link the shader program
    iD = glCreateProgram();
    glAttachShader(iD, vertex);
    glAttachShader(iD, geometry);
    glAttachShader(iD, fragment);
    glLinkProgram(iD);

    //Print linking errors if any
    glGetProgramiv(iD, GL_LINK_STATUS, &success);

    if (success == 0)
    {
        glGetProgramInfoLog(iD, 512, NULL, infoLog);
        std::cerr << "Failed to link shader program: \n" << infoLog << '\n';
    }

    //3.Delete the shaders 
    glDeleteShader(vertex);
    glDeleteShader(geometry);
    glDeleteShader(fragment);
}

unsigned int Shader::getID()
{
    return iD;
}

