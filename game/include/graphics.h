#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "globals.h"

using namespace std;
/**
 * @brief Loads a 3D model using Assimp.
 */
bool loadAssImp(
    const char * path, 
    vector<unsigned short> & indices,
    vector<glm::vec3> & vertices,
    vector<glm::vec2> & uvs,
    vector<glm::vec3> & normals
    );

/**
 * @brief Loads a shader.
 */
GLuint loadShader(const char* VertexSourcePointer, const char* FragmentSourcePointer);

/**
 * @brief Loads a texture from a file.
 */
GLuint loadTexture(const char* imagepath);

/**
 * @brief Initializes shaders.
 */
void shadersInit();

#endif