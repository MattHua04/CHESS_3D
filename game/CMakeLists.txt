cmake_minimum_required(VERSION 3.10)

project(CHESS_3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -O3)

set(COMMON_OUTPUT_DIR ${CMAKE_BINARY_DIR}/output)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
set(GLEW_DIR /usr/local/opt/glew)
find_package(GLEW REQUIRED)
find_library(GLEW_LIBRARY NAMES GLEW glew)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)
find_library(ASSIMP_LIBRARIES NAMES assimp PATHS /opt/homebrew/lib)

find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/SFML)
include_directories(${PROJECT_SOURCE_DIR}/external/SFML/include)
link_directories(${PROJECT_SOURCE_DIR}/external/SFML/lib)

set(STOCKFISH_DIR ${PROJECT_SOURCE_DIR}/external/Stockfish/src)
set(STOCKFISH_EXECUTABLE ${STOCKFISH_DIR}/stockfish)

if(NOT EXISTS ${STOCKFISH_EXECUTABLE})
    add_custom_target(build_stockfish ALL
        COMMAND make build ARCH=apple-silicon
        WORKING_DIRECTORY ${STOCKFISH_DIR}
        COMMENT Building Stockfish
    )
else()
    message(STATUS Stockfish executable found.)
endif()

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
set(SOURCES ${SOURCES})

add_executable(CHESS_3D ${SOURCES})

set_target_properties(CHESS_3D PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${COMMON_OUTPUT_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${COMMON_OUTPUT_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${COMMON_OUTPUT_DIR}/bin
)

target_include_directories(CHESS_3D PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(CHESS_3D PRIVATE
    ${OPENGL_LIBRARIES}
    glfw
    ${GLEW_LIBRARY}
    ${ASSIMP_LIBRARIES}
    nlohmann_json::nlohmann_json
    CURL::libcurl
    sfml-audio
)

target_include_directories(CHESS_3D PRIVATE /usr/local/include /opt/homebrew/include)

if(NOT EXISTS ${STOCKFISH_EXECUTABLE})
    add_dependencies(CHESS_3D build_stockfish)
endif()

file(COPY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${COMMON_OUTPUT_DIR}/bin)
file(COPY ${STOCKFISH_DIR}/stockfish DESTINATION ${COMMON_OUTPUT_DIR}/bin)