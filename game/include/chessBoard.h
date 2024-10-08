#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include "globals.h"
#include "chessPiece.h"
#include "graphics.h"
#include "fen.h"
#include "chessEngine.h"
#include "sound.h"

using namespace std;

// ChessBoard class declaration
class ChessBoard {
public:
    // Constructor
    ChessBoard();
    ChessBoard(glm::vec3 position);

    void printBoard();

    // Function to add a chess piece to the board
    void addPiece(ChessPiece* piece, int x, int y);

    // Function to render the chessboard and pieces
    void render();

    void checkHoveredPieces();

    string getSquareAtCenter();

    void userInteraction();

    void update();

    void startGame() {gameRunning = true;};

    bool getGameRunning() { return gameRunning; }

    string getFEN() const { return FEN; }

    void setFEN(const string& fen) { FEN = fen; }

    // Function to move a piece based on chess notation (e.g., "e2e4")
    void movePiece(const string& move);

    void getOpponentMove();

    void generateBoardMesh();
    void generateCheckerboardTexture();

    void reset();

    bool inCheck(const string& player);
    bool checkMated(const string& player);

private:
    GLuint boardVAO;
    GLuint boardTexture;
    ChessPiece* board[8][8];  // Backing array to store pointers to chess pieces
    vector<ChessPiece*> pieces;  // List of chess pieces for rendering
    glm::vec3 boardPosition;  // Position of the board in 3D space
    glm::mat4 modelMatrix;
    string hoveredPieceLocation;
    string selectedPieceLocation;
    string targetPointerLocation;
    string playerTurn;
    bool opponentProcessing;
    string FEN;
    double checkMatedTime;
    bool whiteInCheck;
    bool blackInCheck;
    bool overrideMode;
    string opponentMove;
    bool opponentMoveReceived;
    bool gameRunning;

    // Helper function to convert notation to board coordinates
    pair<int, int> getPositionFromNotation(const string& notation);

    bool validMove(const string& move, bool errorsOff=false, bool checkCastling=true);
    bool testMove(const string& move);
    bool squareUnderAttackBy(string square, string player);
    bool isValidRookMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff=false);
    bool isValidKnightMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff=false);
    bool isValidBishopMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff=false);
    bool isValidQueenMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff=false);
    bool isValidKingMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff=false, bool checkCastling=true);
    bool isValidPawnMove(int fromRow, int fromCol, int toRow, int toCol, ChessPiece* piece, bool errorsOff=false);
};

extern ChessBoard chessBoard;

#endif