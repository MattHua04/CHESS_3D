#ifndef CHESS_PIECE_H
#define CHESS_PIECE_H

#include "globals.h"
#include "graphics.h"

using namespace std;

/**
 * @class ChessPiece
 * @brief Represents a chess piece.
 */
class ChessPiece {
public:
    /**
     * @brief Initializes the chess piece textures.
     */
    static void init();

    /**
     * @brief Constructor with parameters.
     * @param x The x-coordinate of the piece.
     * @param y The y-coordinate of the piece.
     * @param z The z-coordinate of the piece.
     * @param type The type of the piece.
     * @param player The player that owns the piece.
     */
    ChessPiece(float x, float y, float z, string type, string player);

    /**
     * @brief Converts the piece to a new type.
     * @param newType The new type to convert to.
     */
    void convertTo(const std::string& newType);

    /**
     * @brief Sets the position of the chess piece.
     */
    void setPosition(float x, float y, float z);

    void setHovered(bool hovered) {this->hovered = hovered;};

    /**
     * @brief Returns the position of the chess piece.
     * @return The position of the chess piece.
     */
    glm::vec3 getPosition() const;

    /**
     * @brief Returns the board location of the chess piece.
     * @return The board location of the chess piece.
     */
    string getBoardLocation() const;

    /**
     * @brief Returns whether the piece has moved.
     * @return True if the piece has moved, false otherwise.
     */
    bool getHasMoved() const { return hasMoved; }

    /**
     * @brief Sets whether the piece has moved.
     * @param hasMoved Whether the piece has moved.
     */
    void setHasMoved(bool hasMoved) { this->hasMoved = hasMoved; }

    /**
     * @brief Returns the player that owns the piece.
     * @return The player that owns the piece.
     */
    string getPlayer() const { return player; }

    /**
     * @brief Returns the type of the piece.
     * @return The type of the piece.
     */
    string getType() const { return type; }

    /**
     * @brief Loads the mesh for the chess piece.
     */
    void loadMesh();

    /**
     * @brief Creates a color texture.
     * @param color The color of the texture.
     * @return The texture ID.
     */
    static GLuint createColorTexture(const glm::vec3& color);

    /**
     * @brief Updates the model matrix of the chess piece.
     */
    void updateModelMatrix();

    /**
     * @brief Renders the chess piece.
     */
    void render() const;

    static GLuint blackTexture; // Texture for black pieces
    static GLuint whiteTexture; // Texture for white pieces
    static GLuint greyTexture; // Texture for grey pieces
    static GLuint darkGreyTexture; // Texture for dark grey pieces

private:
    string type; // Type of the piece
    string player; // Player that owns the piece
    bool hovered; // Whether the piece is hovered over
    GLuint pieceVAO; // Vertex array object for the piece
    GLuint pieceTexture; // Texture for the piece
    unsigned int pieceVertexCount; // Number of vertices in the piece mesh
    glm::vec3 position; // Position of the piece in 3D space
    string boardLocation; // Location of the piece on the chessboard
    glm::mat4 modelMatrix; // Model matrix for the piece
    bool hasMoved; // Whether the piece has moved
};

#endif
