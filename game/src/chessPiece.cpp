#include "chessPiece.h"

using namespace std;

// Piece textures
GLuint ChessPiece::blackTexture;
GLuint ChessPiece::whiteTexture;
GLuint ChessPiece::greyTexture;
GLuint ChessPiece::darkGreyTexture;

void ChessPiece::init() {
    blackTexture = createColorTexture(glm::vec3(0.0f, 0.0f, 0.0f));
    whiteTexture = createColorTexture(glm::vec3(1.0f, 1.0f, 1.0f));
    greyTexture = createColorTexture(glm::vec3(0.5f, 0.5f, 0.5f));
    darkGreyTexture = createColorTexture(glm::vec3(0.2f, 0.2f, 0.2f));
}

void ChessPiece::convertTo(const std::string& newType) {
    this->type = newType;
    loadMesh();
    pieceTexture = (player == "white") ? whiteTexture : blackTexture;
    updateModelMatrix();
}

void ChessPiece::setPosition(float x, float y, float z) {
    float squareSize = 0.3f;
    glm::vec3 a0(squareSize * 4, 0, -squareSize * 4);
    glm::vec3 piecePosition(-squareSize * x - squareSize / 2, 0, squareSize * y + squareSize / 2);
    this->position = a0 + piecePosition;
    boardLocation = string(1, 'a' + x) + to_string(int(y + 1));
    updateModelMatrix();
}

void ChessPiece::updateModelMatrix() {
    modelMatrix = glm::translate(glm::mat4(1.0f), position);
    // Rotate white pieces
    if (player == "white") {
        modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

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
    taken = false;
}

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

    // Create VAO
    glGenVertexArrays(1, &pieceVAO);
    glBindVertexArray(pieceVAO);

    // Vertex buffer
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // UV buffer
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Normal buffer
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(2);

    // Unbind VAO
    glBindVertexArray(0);

    pieceVertexCount = indices.size();
}

GLuint ChessPiece::createColorTexture(const glm::vec3& color) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    unsigned char pixelData[3] = { static_cast<unsigned char>(color.r * 255), 
                                    static_cast<unsigned char>(color.g * 255), 
                                    static_cast<unsigned char>(color.b * 255) };
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return textureID;
}

void ChessPiece::render() const {
    glUseProgram(shaderProgram);

    // Set up view matrices
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * modelMatrix;
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, glm::value_ptr(ViewMatrix));

    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera.getPosition()));

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    GLuint useTexture;
    if (player == "white") {
        useTexture = (hovered && !taken) ? greyTexture : pieceTexture;
    } else {
        useTexture = (hovered && !taken) ? darkGreyTexture : pieceTexture;
    }

    glBindTexture(GL_TEXTURE_2D, useTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "textureSampler"), 0);

    // Bind VAO
    glBindVertexArray(pieceVAO);
    glDrawArrays(GL_TRIANGLES, 0, pieceVertexCount);
}

void ChessPiece::animateMove(float x, float y, float z, bool& animating) {
    glm::vec3 startPosition = position;
    glm::vec3 endPosition = glm::vec3(x, y, z);
    int steps = 100;
    glm::vec3 a0(0.3f * 4, 0, -0.3f * 4);
    glm::vec3 adjustedEndPosition = a0 + glm::vec3(-0.3f * x - 0.3f / 2, 0, 0.3f * y + 0.3f / 2);
    for (int i = 0; i <= steps; ++i) {
        float t = static_cast<float>(i) / steps;
        glm::vec3 intermediatePosition = glm::mix(startPosition, adjustedEndPosition, t);
        // Knight moves in a curve
        if (type == "knight") {
            float height = 0.5f * sin(glm::pi<float>() * t);
            intermediatePosition.y += height;
        }
        position = intermediatePosition;
        updateModelMatrix();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    animating = false;
}