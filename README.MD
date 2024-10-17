<h1 align="center">GT ECE 4122 Final Project Fall 2024</h1>

<p align="center">
 <a href="https://youtu.be/F0FKCEGGE0o">
  <img src="https://github.com/MattHua04/CHESS_3D/blob/main/CHESS_3D.png" alt="Cover Image" width="100%">
 </a>
Click for Demo Video
</p>

## Overview

A 3D single or multiplayer player chess game created with OpenGL. Stockfish was used as the opponent with options to process moves locally or with a public REST API. Sound effects and music were implemented using SFML. TCP was used to support multiplayer games.

## Requirements
### Preferably installed at system level:
* __OpenGL__
* __GLFW__
* __GLEW__
* __GLM__
* __AssImp__
* __Curl__
* __JSON__
### Provided in this project:
* SFML (2.6.x branch)
* Stockfish
    * At large depth settings and especially on limited hardware, Stockfish may have an unreasonable response time. Try setting the processing pipeline to remote if required

## Setup/Build/Run
__This project was developed on MacOS Apple Silicon, limited testing has been done in other environments.__
1. Install/configure dependencies:
    * __Mac__: 
```brew install glfw glew glm assimp curl nlohmann-json```

    * __Windows__: 
Unfortunately, the source code for Stockfish and multiplayer support will not compile on Windows machines.

    * __Linux__: 
```sudo apt install libglfw3-dev libglew-dev libglm-dev assimp-utils libcurl4-openssl-dev nlohmann-json3-dev```

    * A zip folder with provided dependencies has also been provided. If you choose to use it then extract the contents to __external__ and make the alternate CMakeLists.txt files active:
        * [Alternate CMakeLists.txt file in /game](game/AlternateCMakeLists.txt)
        * [Alternate CMakeLists.txt file in /game/external](game/external/AlternateCMakeLists.txt)

2. Aquire assets/setup configurations:
    * Download [background music](game/assets/audio/background.mp3)
        * Either manually download it from this repo or use ```git lfs pull```

    * Rename [config.txt](game/include/config.txt) to ```config.h``` and replace ```<SERVER_IP_ADDRESS>``` with the IP address of the machine you choose to host the server on. Ensure the ```PORT``` number has permissions set correctly and configure port forwarding if public access is required.

3. Build:
    * Execute in the __build__ directory: ```cmake .. && make``` or ```cmake .. && cmake --build .```

    * If using system level packages doesn't build properly, try the provided packages.

4. Run:
    * Execute in the __build/output/bin__ directory: ```./CHESS_3D```

        * ```-width <window width>```: optional manual window width (defaults to 1024 px)
        * ```-diff <difficulty>```: optional manual Stockfish difficulty [0, 20] (only applies to local Stockfish, defaults to 10)
        * ```-depth <processing depth>```: optional manual processing depth for Stockfish (defaults to 10)
        * ```-remote```: optional flag to use the Stockfish REST API instead of running Stockfish locally
        * ```-multiplayer```: optional flag to attempt joining a multiplayer match

    * To start the multiplayer server, execute in the [server source directory](server/src/): ```g++ main.cpp -o main && ./main```
