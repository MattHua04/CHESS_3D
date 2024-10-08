#include "chessPiece.h"

using namespace std;

GLuint ChessPiece::blackTexture;
GLuint ChessPiece::whiteTexture;
GLuint ChessPiece::greyTexture;

void ChessPiece::init() {
    blackTexture = createColorTexture(glm::vec3(0.0f, 0.0f, 0.0f)); // Black
    whiteTexture = createColorTexture(glm::vec3(1.0f, 1.0f, 1.0f)); // White
    greyTexture = createColorTexture(glm::vec3(0.5f, 0.5f, 0.5f)); // Grey
}

void ChessPiece::convertTo(const std::string& newType) {
    this->type = newType;
    loadMesh();
    pieceTexture = (player == "white") ? whiteTexture : blackTexture;
    updateModelMatrix();
}

// Setters
void ChessPiece::setPosition(float x, float y, float z) {
    // Assuming each square is 1 unit wide and boardPosition is the origin of the board
    float squareSize = 0.3f;
    glm::vec3 a0(squareSize * 4, 0, -squareSize * 4);
    glm::vec3 piecePosition(-squareSize * x - squareSize / 2, 0, squareSize * y + squareSize / 2);
    this->position = a0 + piecePosition;  // Store the 3D position of the piece
    boardLocation = string(1, 'a' + x) + to_string(int(y + 1));
    updateModelMatrix();
}

void ChessPiece::updateModelMatrix() {
    modelMatrix = glm::translate(glm::mat4(1.0f), position);
    if (player == "white") {
        // Translate to origin, rotate, then translate back
        modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

// Constructor
ChessPiece::ChessPiece(float x, float y, float z, string type, string player) {
    setPosition(x, y, z);
    this->type = type;
    this->player = player;
    hovered = false;
    hasMoved = false;
    boardLocation = string(1, 'a' + x) + to_string(int(y + 1));
    loadMesh();
    pieceTexture = (player == "white") ? whiteTexture : blackTexture;
    modelMatrix = glm::mat4(1.0f);
    updateModelMatrix();
}

// Getters
glm::vec3 ChessPiece::getPosition() const {
    return position;
}

string ChessPiece::getBoardLocation() const {
    return boardLocation;
}

void ChessPiece::loadMesh() {
    vector<unsigned short> indices;
    vector<glm::vec3> vertices;
    vector<glm::vec2> uvs;
    vector<glm::vec3> normals;
    bool res = loadAssImp(("assets/models/" + type + ".obj").c_str(), indices, vertices, uvs, normals);
    if (!res) {
        cerr << "Error loading " << type << " mesh!" << endl;
        return;
    }

    // Create the VAO and VBOs
    glGenVertexArrays(1, &pieceVAO);
    glBindVertexArray(pieceVAO);

    // Create the vertex buffer
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Create the UV buffer
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Create the normal buffer
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(2);

    // Unbind the VAO
    glBindVertexArray(0);

    // Store the vertex count (use indices for drawing)
    pieceVertexCount = indices.size();
}

GLuint ChessPiece::createColorTexture(const glm::vec3& color) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Create a 1x1 pixel texture filled with the specified color
    unsigned char pixelData[3] = { static_cast<unsigned char>(color.r * 255), 
                                    static_cast<unsigned char>(color.g * 255), 
                                    static_cast<unsigned char>(color.b * 255) };
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return textureID;
}

void ChessPiece::render() const {
    // Choose the shader program based on the player color
    glUseProgram(shaderProgram);

    // Set up lighting and view matrices
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * modelMatrix;
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, glm::value_ptr(ViewMatrix));

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    GLuint useTexture = (player == "white" && hovered) ? greyTexture : pieceTexture;
    glBindTexture(GL_TEXTURE_2D, useTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "textureSampler"), 0);

    // Bind the vertex array object (VAO) and draw the mesh
    glBindVertexArray(pieceVAO);
    glDrawArrays(GL_TRIANGLES, 0, pieceVertexCount);
}