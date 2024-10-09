<h1 align="center">GT ECE 4122 Final Project Fall 2024</h1>

<p align="center">
 <a href="https://youtu.be/PhItT7dz1O8">
  <img src="https://github.com/MattHua04/CHESS_3D/blob/main/CHESS_3D.png" alt="Cover Image" width="100%">
 </a>
</p>

## Overview

A 3D single player chess game created with OpenGL. Stockfish was used as the opponent with options to process moves locally or with a public REST API. Sound effects and music were implemented using SFML.

## Requirements
### Preferrably installed at system level:
* __OpenGL__
* __GLFW__
* __GLEW__
* __GLM__
* __AssImp__
* __Curl__
* __JSON__
### Provided in project:
* SFML
* Stockfish
 * At large depth settings and especially on limited hardware, Stockfish may take an unreasonable time to process moves. Try setting the processing pipeline to remote if required

## Setup/Build
1. Install/configure dependencies:
 * Mac: ```brew install glfw glew glm assimp curl nlohmann-json```
 * Windows: ```git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install glfw3 glew glm assimp curl nlohmann-json
.\vcpkg\vcpkg integrate install
```
 * Linux: ```sudo apt install libglfw3-dev libglew-dev libglm-dev assimp-utils libcurl4-openssl-dev nlohmann-json3-dev```
 * A zip folder with provided dependencies has also been provided. If you choose to use it then extract the contents to /external and make the alternate CMakeLists.txt files active:
  * [Alternate CMakeLists.txt file in /game](game/AlternateCMakeLists.txt)
  * [Alternate CMakeLists.txt file in /game/external](game/external/AlternateCMakeLists.txt)
2. 
