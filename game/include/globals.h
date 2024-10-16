#ifndef GLOBALS_H
#define GLOBALS_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <utility>
#include <filesystem>
#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <SFML/Audio.hpp>
#include "camera.h"
#include "config.h"

using namespace std;

extern float WIDTH;
extern float HEIGHT;
extern GLFWwindow* window;
extern bool remote;
extern atomic<bool> multiplayer;
extern string playerColor;
extern atomic<bool> resetBoard;

extern GLuint frameBuffer;
extern GLuint frameVAO;
extern GLuint frameVBO;
extern GLuint frameBufferTexture;

extern GLuint logoTexture;
extern int logoTextureWidth;
extern int logoTextureHeight;

extern GLuint startTextTexture;
extern int startTextureWidth;
extern int startTextureHeight;

extern GLuint shaderProgram;
extern GLuint frameShaderProgram;

extern GLuint MatrixID;
extern GLuint ViewMatrixID;
extern GLuint ModelMatrixID;

extern glm::mat4 ProjectionMatrix;
extern glm::mat4 ViewMatrix;
extern glm::mat4 ModelMatrix;
extern glm::mat4 MVP;

#endif