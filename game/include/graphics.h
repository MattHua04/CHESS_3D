#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "globals.h"

using namespace std;

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

/**
 * @brief Represents a packed vertex.
 */
struct PackedVertex{
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	bool operator<(const PackedVertex that) const{
		return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
	};
};

/**
 * @brief Checks if a similar vertex exists in the output index.
 */
bool getSimilarVertexIndex_fast( 
	PackedVertex & packed, 
	map<PackedVertex,unsigned short> & VertexToOutIndex,
	unsigned short & result
);

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
 * @brief Indexes the VBO.
 */
void indexVBO(
    vector<glm::vec3> & in_vertices,
    vector<glm::vec2> & in_uvs,
    vector<glm::vec3> & in_normals,
    vector<unsigned short> & out_indices,
    vector<glm::vec3> & out_vertices,
    vector<glm::vec2> & out_uvs,
    vector<glm::vec3> & out_normals
    );

/**
 * @brief Loads a shader from a file.
 */
GLuint loadShader(const char* VertexSourcePointer, const char* FragmentSourcePointer);

/**
 * @brief Loads a texture from a file.
 */
GLuint loadTexture(const char* imagepath);

/**
 * @brief Cleans up the graphics.
 */
void shadersInit();

#endif