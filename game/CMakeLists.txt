cmake_minimum_required(VERSION 3.10)

project(CHESS_3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -O3)

set(COMMON_OUTPUT_DIR ${CMAKE_BINARY_DIR}/output)

if (APPLE)
    set(GLEW_DIR /usr/local/opt/glew)
    set(ASSIMP_LIBRARIES /opt/homebrew/lib/libassimp.dylib)
elseif (WIN32)
    message(STATUS "Using vcpkg for dependencies on Windows.")
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif (UNIX AND NOT APPLE)
    message(STATUS "Configuring for Linux")
endif()

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/SFML)
include_directories(${PROJECT_SOURCE_DIR}/external/SFML/include)
link_directories(${PROJECT_SOURCE_DIR}/external/SFML/lib)

set(STOCKFISH_DIR ${PROJECT_SOURCE_DIR}/external/Stockfish/src)
set(STOCKFISH_EXECUTABLE ${STOCKFISH_DIR}/stockfish)

if(NOT EXISTS ${STOCKFISH_EXECUTABLE})
    if (APPLE)
        add_custom_target(build_stockfish ALL
            COMMAND make build ARCH=apple-silicon
            WORKING_DIRECTORY ${STOCKFISH_DIR}
            COMMENT "Building Stockfish for macOS"
        )
    elseif (WIN32)
        add_custom_target(build_stockfish ALL
            COMMAND mingw32-make build ARCH=x86-64
            WORKING_DIRECTORY ${STOCKFISH_DIR}
            COMMENT "Building Stockfish for Windows"
        )
    endif()
else()
    message(STATUS "Stockfish executable found.")
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
    GLEW::GLEW
    assimp::assimp
    nlohmann_json::nlohmann_json
    CURL::libcurl
    sfml-audio
)

if (APPLE)
    target_include_directories(CHESS_3D PRIVATE /usr/local/include /opt/homebrew/include)
endif()

if(NOT EXISTS ${STOCKFISH_EXECUTABLE})
    add_dependencies(CHESS_3D build_stockfish)
endif()

file(COPY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${COMMON_OUTPUT_DIR}/bin)
file(COPY ${STOCKFISH_DIR}/stockfish DESTINATION ${COMMON_OUTPUT_DIR}/bin)