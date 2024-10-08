#ifndef CHESS_PIECE_H
#define CHESS_PIECE_H

#include "globals.h"
#include "graphics.h"

using namespace std;

class ChessPiece {
public:
    static void init();

    // Constructor
    ChessPiece(float x, float y, float z, string type, string player);

    void convertTo(const std::string& newType);

    // Setters
    void setPosition(float x, float y, float z);

    void setHovered(bool hovered) {this->hovered = hovered;};

    // Getters
    glm::vec3 getPosition() const;

    string getBoardLocation() const;

    bool getHasMoved() const { return hasMoved; }

    void setHasMoved(bool hasMoved) { this->hasMoved = hasMoved; }

    string getPlayer() const { return player; }
    string getType() const { return type; }

    void loadMesh();
    static GLuint createColorTexture(const glm::vec3& color);

    void updateModelMatrix();

    // Render the chess piece
    void render() const;

    static GLuint blackTexture;
    static GLuint whiteTexture;
    static GLuint greyTexture;

private:
    // Mesh VAO and vertex count for this chess piece
    string type;
    string player;
    bool hovered;
    GLuint pieceVAO;
    GLuint pieceTexture;
    unsigned int pieceVertexCount;
    glm::vec3 position;
    string boardLocation;
    glm::mat4 modelMatrix;
    bool hasMoved;
};

#endif
