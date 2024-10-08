#include "globals.h"
#include "graphics.h"
#include "sound.h"
#include "camera.h"
#include "fen.h"
#include "chessEngine.h"
#include "chessPiece.h"
#include "chessBoard.h"

using namespace std;

float aspectRatio = 4.0/3.0;
float WIDTH = 1024.0f;
float HEIGHT = WIDTH / aspectRatio;
GLFWwindow* window;
GLuint frameBuffer;
GLuint frameVAO;
GLuint frameBufferTexture;
GLuint logoTexture;
GLuint startTextTexture;

// Matrix uniform IDs for each shader
GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;

glm::mat4 ProjectionMatrix;
glm::mat4 ViewMatrix;
glm::mat4 ModelMatrix;
glm::mat4 MVP;

Camera camera;
static float lastX = WIDTH / 2.0f;
static float lastY = HEIGHT / 2.0f;

// Defaults
int depth = 10;
int difficulty = 10; // 0-20
bool remote = false;
Stockfish stockfish;
ChessBoard board;

int readArgs(int argc, char* argv[]) {
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-depth" && i + 1 < argc) {
            depth = stoi(argv[++i]);  // Convert to integer
        } else if (arg == "-diff" && i + 1 < argc) {
            difficulty = stoi(argv[++i]);  // Convert to integer
            if (difficulty < 0 || difficulty > 20) {
                difficulty = min(max(difficulty, 0), 20);
                cerr << "Difficulty must be between 0 and 20! Defaulting to " << to_string(difficulty) << endl;
            }
        } else if (arg == "-remote") {
            remote = true;
        } else if (arg == "-width") {
            if (i + 1 < argc) {
                WIDTH = max(stof(argv[++i]), 0.0f);
                HEIGHT = WIDTH / aspectRatio;
            } else {
                cerr << "Missing argument for -width, defaulting to " << WIDTH << endl;
            }
        } else {
            cerr << "Unknown argument: " << arg << endl;
            cerr << "Usage: " << argv[0] << " [-width <window width>] [-depth <depth>] [-diff <difficulty>]" << endl;
            return 1;
        }
    }
    lastX = WIDTH / 2.0f;
    lastY = HEIGHT / 2.0f;
    return 0;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void errorCallback(int error, const char* description) {
    cerr << "Error: " << description << endl;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true; // Initial state for the first mouse input

    if (firstMouse) {
        camera.processMouseMovement(xpos, ypos);
        firstMouse = false;
    } else {
        camera.processMouseMovement(xpos, ypos);
    }
}

void processInput(GLFWwindow* window, Camera& camera) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.moveCamera(4); // Move up
    } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.moveCamera(0); // Move forward
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.moveCamera(5); // Move down
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.moveCamera(1); // Move backward
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.moveCamera(2); // Move left
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.moveCamera(3); // Move right
    }
}

int generalInit() {
    // Initialize GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return 1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( WIDTH, HEIGHT, "CHESS 3D", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
    glfwSetErrorCallback(errorCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW.\n");
		getchar();
		glfwTerminate();
		return 1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, WIDTH/2.0f, HEIGHT/2.0f);

	glClearColor(0.6f, 0.6f, 0.6f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it is closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

    // Set up camera
    camera = Camera(glm::vec3(0.0f, 3.0f, -2.5f), glm::vec3(0.0f, 0.0f, 0.0f), 90.0f, -50.0f);

    // Load shaders
    shadersInit();

    // Retrieve uniform locations for shader
    MatrixID = glGetUniformLocation(shaderProgram, "MVP");
    ViewMatrixID = glGetUniformLocation(shaderProgram, "V");
    ModelMatrixID = glGetUniformLocation(shaderProgram, "M");

    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    GLuint renderBuffer;
    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

    glGenTextures(1, &frameBufferTexture);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);

    // Load overlay textures
    logoTexture = loadTexture("assets/textures/logo.png");
    startTextTexture = loadTexture("assets/textures/start.png");

    // Unbind the texture to prevent accidental modification
    glBindTexture(GL_TEXTURE_2D, 0);

    // Bind the framebuffer back to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cerr << "Error: Framebuffer is not complete!" << endl;
    }

    // Set up the quad for rendering the framebuffer texture
    float FBOQuadVertices[] = {
        // Positions   // Texture Coords
        -1.0f,  1.0f,  0.0f, 1.0f, // Top-left
        -1.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
        1.0f, -1.0f,  1.0f, 0.0f, // Bottom-right
        1.0f,  1.0f,  1.0f, 1.0f  // Top-right
    };

    // Index order for drawing the quad as two triangles
    unsigned int FBOQuadIndices[] = {
        0, 1, 2,  // First triangle
        0, 2, 3   // Second triangle
    };
    GLuint vbo, ebo;
    glGenVertexArrays(1, &frameVAO);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(frameVAO);
    // VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FBOQuadVertices), FBOQuadVertices, GL_STATIC_DRAW);
    // EBO for indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(FBOQuadIndices), FBOQuadIndices, GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    return 0;
}

void cleanup() {
    glDeleteProgram(shaderProgram);
    glDeleteProgram(frameShaderProgram);
    glfwTerminate();
}

void render(string overlayContent="") {
    // Clear the color and depth buffers
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, WIDTH, HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    board.render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // // Bind the texture.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Use the shader program for the quad
    glUseProgram(frameShaderProgram);
    // Bind the FBO texture
    glUniform1i(glGetUniformLocation(frameShaderProgram, "screenTexture"), 0);
    float aspectRatio = static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
    glUniform1f(glGetUniformLocation(frameShaderProgram, "aspectRatio"), aspectRatio);
    glUniform1i(glGetUniformLocation(frameShaderProgram, "whiteInCheck"), board.inCheck("white"));
    glUniform1i(glGetUniformLocation(frameShaderProgram, "whiteMated"), board.checkMated("white"));
    glUniform1i(glGetUniformLocation(frameShaderProgram, "blackMated"), board.checkMated("black"));
    glUniform1i(glGetUniformLocation(frameShaderProgram, "gameRunning"), board.getGameRunning());

    if (overlayContent == "start") {
        glUniform1i(glGetUniformLocation(frameShaderProgram, "useOverlayTexture"), GL_TRUE);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, startTextTexture);
        glUniform1i(glGetUniformLocation(frameShaderProgram, "overlayTexture"), 1);
    } else if (overlayContent == "logo") {
        glUniform1i(glGetUniformLocation(frameShaderProgram, "useOverlayTexture"), GL_TRUE);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, logoTexture);
        glUniform1i(glGetUniformLocation(frameShaderProgram, "overlayTexture"), 1);
    } else {
        glUniform1i(glGetUniformLocation(frameShaderProgram, "useOverlayTexture"), GL_FALSE);
    }

    // Render the quad
    glBindVertexArray(frameVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Swap buffers
    glfwSwapBuffers(window);
    
    // Poll for and process events
    glfwPollEvents();
}

int main(int argc, char* argv[]) {
    if (readArgs(argc, argv)) {
        return 1;
    }
    if(generalInit()) {
        return 1;
    };
    initSound();
    stockfish.init();
    stockfish.setRemoteProcessing(remote);
    stockfish.setDepth(depth);
    stockfish.setDifficulty(difficulty);
    ChessPiece::init();
    board = ChessBoard(glm::vec3(0.0f, 0.0f, 0.0f));

    auto start = chrono::high_resolution_clock::now();
    while (chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - start).count() < 2) {
        if (!board.getGameRunning() && glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {
            return 0;
        }
        render("logo");
    }

    while (!glfwWindowShouldClose(window) ) {
        if (!board.getGameRunning() && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            board.startGame();
            while (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {glfwPollEvents();}
            startSound.play();
        } else if (board.getGameRunning() && glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {
            board.reset();
            while (glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {glfwPollEvents();}
            checkmateSound.play();
        } else if (!board.getGameRunning() && glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {
            break;
        }

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        // Process mouse movement
        camera.processMouseMovement(xoffset, yoffset);
        processInput(window, camera);

        ProjectionMatrix = camera.getProjectionMatrix();
        ViewMatrix = camera.getViewMatrix();
        ModelMatrix = glm::mat4(1.0f);
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        
        board.update();

        render((board.getGameRunning()) ? "" : "start");
    }
    
    cleanup();
    return 0;
}