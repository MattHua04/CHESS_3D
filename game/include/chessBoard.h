#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "globals.h"
#include "chessPiece.h"
#include "graphics.h"
#include "fen.h"
#include "chessEngine.h"
#include "sound.h"
#include "multiplayer.h"

using namespace std;

/**
 * @class ChessBoard
 * @brief Implements all functionalities of a chessboard.
 */
class ChessBoard {
public:
    /**
     * @brief Default constructor.
     */
    ChessBoard();

    /**
     * @brief Constructor with parameters.
     * @param position The position of the chessboard.
     */
    ChessBoard(glm::vec3 position);

    /**
     * @brief Prints the current state of the chessboard.
     */
    void printBoard();

    /**
     * @brief Adds a piece to the chessboard.
     * @param piece The piece to add.
     * @param x The x-coordinate of the piece.
     * @param y The y-coordinate of the piece.
     */
    void addPiece(ChessPiece* piece, int x, int y);

    /**
     * @brief Renders the chessboard and pieces.
     */
    void render();

    /**
     * @brief Checks if a piece is hovered over by the mouse.
     */
    void checkHoveredPieces();

    /**
     * @brief Returns the square at the center of the screen.
     * @return The square notation.
     */
    string getSquareAtCenter();

    /**
     * @brief Handles user interaction with the chessboard.
     */
    void userInteraction();

    /**
     * @brief Updates the chessboard during the game loop.
     */
    void update();

    /**
     * @brief Starts the game.
     */
    void startGame() {gameRunning = true;};

    /**
     * @brief Returns whether the game is running.
     * @return True if the game is running, false otherwise.
     */
    bool getGameRunning() { return gameRunning; }

    /**
     * @brief Returns the FEN string of the chessboard.
     * @return The FEN string.
     */
    string getFEN() const { return FEN; }

    /**
     * @brief Sets the FEN string of the chessboard.
     * @param fen The FEN string.
     */
    void setFEN(const string& fen) { FEN = fen; }

    /**
     * @brief Moves a piece on the chessboard.
     * @param move The move to make.
     */
    void movePiece(const string& move, bool sendMoveToMultiplayerOpponent=true);

    /**
     * @brief Gets the move from the opponent (Stockfish).
     */
    void getOpponentMove();

    /**
     * @brief Generates the mesh for the chessboard.
     */
    void generateBoardMesh();

    /**
     * @brief Generates the texture for the chessboard.
     */
    void generateCheckerboardTexture();

    /**
     * @brief Resets the chessboard to the starting position.
     */
    void reset();

    /**
     * @brief Checks if a player is in check.
     * @param player The player to check.
     * @return True if the player is in check, false otherwise.
     */
    bool inCheck(const string& player);

    /**
     * @brief Checks if a player is checkmated.
     * @param player The player to check.
     * @return True if the player is checkmated, false otherwise.
     */
    bool checkMated(const string& player);

private:
    GLuint boardVAO; // Vertex array object for the chessboard
    GLuint boardTexture; // Texture for the chessboard
    ChessPiece* board[8][8]; // Backing array to store pointers to chess pieces
    vector<ChessPiece*> pieces; // List of chess pieces for rendering
    glm::vec3 boardPosition; // Position of the board in 3D space
    glm::mat4 modelMatrix; // Model matrix for the board
    string hoveredPieceLocation; // Location of the piece currently hovered over
    string selectedPieceLocation; // Location of the piece currently selected
    string targetPointerLocation; // Location of the square at the center of the screen
    string playerTurn; // The current player's turn
    bool opponentProcessing; // Flag to indicate if the opponent is processing a move
    string FEN; // FEN string of the chessboard
    double checkMatedTime; // Time when the checkmate occurred
    bool whiteInCheck; // Flag to indicate if white is in check
    bool blackInCheck; // Flag to indicate if black is in check
    bool overrideMode; // Flag to override the player's turn
    string opponentMove; // The move received from the opponent
    bool opponentMoveReceived; // Flag to indicate if the opponent's move has been received
    bool gameRunning; // Flag to indicate if the game is running

    /**
     * @brief Checks if a square is under attack by a player.
     * @param square The square to check.
     * @param player The player to check.
     * @return True if the square is under attack, false otherwise.
     */
    pair<int, int> getPositionFromNotation(const string& notation);

    /**
     * @brief Checks if a square is under attack by a player.
     * @param square The square to check.
     * @param player The player to check.
     * @return True if the square is under attack, false otherwise.
     */
    bool validMove(const string& move, bool errorsOff=false, bool checkCastling=true);

    /**
     * @brief Checks if a move is valid.
     * @param move The move to check.
     * @return True if the move is valid, false otherwise.
     */
    bool testMove(const string& move);

    /**
     * @brief Checks if a square is under attack by a player.
     * @param square The square to check.
     * @param player The player to check.
     * @return True if the square is under attack, false otherwise.
     */
    bool squareUnderAttackBy(string square, string player);

    /**
     * @brief Checks if a player is in check.
     * @param player The player to check.
     * @return True if the player is in check, false otherwise.
     */
    bool isValidRookMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff=false);

    /**
     * @brief Checks if a player is in check.
     * @param player The player to check.
     * @return True if the player is in check, false otherwise.
     */
    bool isValidKnightMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff=false);

    /**
     * @brief Checks if a player is in check.
     * @param player The player to check.
     * @return True if the player is in check, false otherwise.
     */
    bool isValidBishopMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff=false);

    /**
     * @brief Checks if a player is in check.
     * @param player The player to check.
     * @return True if the player is in check, false otherwise.
     */
    bool isValidQueenMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff=false);

    /**
     * @brief Checks if a player is in check.
     * @param player The player to check.
     * @return True if the player is in check, false otherwise.
     */
    bool isValidKingMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff=false, bool checkCastling=true);

    /**
     * @brief Checks if a player is in check.
     * @param player The player to check.
     * @return True if the player is in check, false otherwise.
     */
    bool isValidPawnMove(int fromRow, int fromCol, int toRow, int toCol, ChessPiece* piece, bool errorsOff=false);
};

#endif