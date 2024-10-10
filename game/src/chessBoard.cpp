#include "chessBoard.h"

using namespace std;

ChessBoard::ChessBoard()
    : boardVAO(0), boardTexture(0), boardPosition(glm::vec3(0.0f, 0.0f, 0.0f)), hoveredPieceLocation(""),
    selectedPieceLocation(""), targetPointerLocation(""), playerTurn("white"),
    FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), opponentProcessing(false),
    checkMatedTime(0), whiteInCheck(false), blackInCheck(false), opponentMove(""), opponentMoveReceived(false),
    gameRunning(false), overrideMode(false) {

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j] = nullptr;
        }
    }
}

void ChessBoard::printBoard() {
    cout << "  a b c d e f g h" << endl;
    cout << "  ---------------" << endl;
    for (int i = 7; i >= 0; --i) {
        cout << i+1 << "|";
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] == nullptr) {
                cout << " ";
            } else {
                cout << board[i][j]->getType()[0];
            }
            cout << "|";
        }
        cout << endl;
        cout << "  ---------------" << endl;
    }
}

ChessBoard::ChessBoard(glm::vec3 position)
    : boardVAO(0), boardTexture(0), boardPosition(position), hoveredPieceLocation(""),
    selectedPieceLocation(""), targetPointerLocation(""), playerTurn("white"),
    FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), opponentProcessing(false),
    checkMatedTime(0), whiteInCheck(false), blackInCheck(false), opponentMove(""), opponentMoveReceived(false),
    gameRunning(false), overrideMode(false) {
    
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j] = nullptr;
        }
    }

    // Initialize the white pieces
    addPiece(new ChessPiece(0, 0, 0, "rook", "white"), 0, 0);
    addPiece(new ChessPiece(1, 0, 0, "knight", "white"), 1, 0);
    addPiece(new ChessPiece(2, 0, 0, "bishop", "white"), 2, 0);
    addPiece(new ChessPiece(3, 0, 0, "queen", "white"), 3, 0);
    addPiece(new ChessPiece(4, 0, 0, "king", "white"), 4, 0);
    addPiece(new ChessPiece(5, 0, 0, "bishop", "white"), 5, 0);
    addPiece(new ChessPiece(6, 0, 0, "knight", "white"), 6, 0);
    addPiece(new ChessPiece(7, 0, 0, "rook", "white"), 7, 0);

    for (int i = 0; i < 8; ++i) {
        addPiece(new ChessPiece(i, 1, 0, "pawn", "white"), i, 1);
    }

    // Initialize the black pieces
    addPiece(new ChessPiece(0, 7, 0, "rook", "black"), 0, 7);
    addPiece(new ChessPiece(1, 7, 0, "knight", "black"), 1, 7);
    addPiece(new ChessPiece(2, 7, 0, "bishop", "black"), 2, 7);
    addPiece(new ChessPiece(3, 7, 0, "queen", "black"), 3, 7);
    addPiece(new ChessPiece(4, 7, 0, "king", "black"), 4, 7);
    addPiece(new ChessPiece(5, 7, 0, "bishop", "black"), 5, 7);
    addPiece(new ChessPiece(6, 7, 0, "knight", "black"), 6, 7);
    addPiece(new ChessPiece(7, 7, 0, "rook", "black"), 7, 7);

    for (int i = 0; i < 8; ++i) {
        addPiece(new ChessPiece(i, 6, 0, "pawn", "black"), i, 6);
    }

    generateBoardMesh();
    generateCheckerboardTexture();
}

void ChessBoard::addPiece(ChessPiece* piece, int x, int y) {
    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
        piece->setPosition(x, y, 0);
        pieces.push_back(piece);
        if (board[y][x] != nullptr) {
            cout << "Warning: Overwriting existing piece at (" << x << ", " << y << ")" << endl;
            delete board[x][y];
        }
        board[y][x] = piece;
    } else {
        cerr << "Invalid position for chess piece: (" << x << ", " << y << ")" << endl;
    }
}

void ChessBoard::render() {
    glUseProgram(shaderProgram);
    glDisable(GL_CULL_FACE);

    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * modelMatrix;
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, glm::value_ptr(ViewMatrix));

    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera.getPosition()));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, boardTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "textureSampler"), 0);

    glBindVertexArray(boardVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Render all the chess pieces
    glEnable(GL_CULL_FACE);
    for (const auto& piece : pieces) {
        piece->render();
    }
}

pair<int, int> ChessBoard::getPositionFromNotation(const string& notation) {
    int file = notation[0] - 'a';  // Convert 'a' to 'h' to 0-7 (column)
    int rank = notation[1] - '1';  // Convert '1' to '8' to 0-7 (row)
    return {file, rank};
}

bool ChessBoard::inCheck(const string& player) {
    // Find the king's position
    int kingRow = -1;
    int kingCol = -1;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] != nullptr && board[i][j]->getType() == "king" && board[i][j]->getPlayer() == player) {
                kingRow = i;
                kingCol = j;
                break;
            }
        }
    }

    if (kingRow == -1 || kingCol == -1) {
        cerr << "Could not find the " << player << " king." << endl;
        return false;
    }
    
    // Check if any opponent pieces can attack the king
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] != nullptr && board[i][j]->getPlayer() == ((player == "white") ? "black" : "white")) {
                string kingPosition = string(1, 'a' + kingCol) + to_string(kingRow + 1);
                string move = board[i][j]->getBoardLocation() + kingPosition;
                if (validMove(move, true)) {
                    whiteInCheck = player == "white";
                    blackInCheck = player == "black";
                    return true;
                }
            }
        }
    }

    if (player == "white") whiteInCheck = false;
    if (player == "black") blackInCheck = false;

    return false;
}

bool ChessBoard::checkMated(const string& player) {
    // Check if there are any valid moves for the player
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] != nullptr && board[i][j]->getPlayer() == player) {
                for (int k = 0; k < 8; ++k) {
                    for (int l = 0; l < 8; ++l) {
                        string move = board[i][j]->getBoardLocation() + string(1, 'a' + l) + to_string(k + 1);
                        if (testMove(move)) {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool ChessBoard::validMove(const string& move, bool errorsOff, bool checkCastling) {
    if (move.length() != 4) return false;

    int fromCol = move[0] - 'a';
    int fromRow = move[1] - '0' - 1;
    int toCol = move[2] - 'a';
    int toRow = move[3] - '0' - 1;

    ChessPiece* piece = board[fromRow][fromCol];
    if (piece == nullptr) {
        cerr << "No piece at source position: " << move.substr(0, 2) << endl;
        return false;
    }

    // Check if it is the player's turn
    if (!errorsOff && playerTurn != piece->getPlayer()) {
        cerr << "It is the opponent's turn." << endl;
        return false;
    }

    // Validate the move based on the piece type
    if (piece->getType() == "pawn") {
        return isValidPawnMove(fromRow, fromCol, toRow, toCol, piece, errorsOff);
    } else if (piece->getType() == "knight") {
        return isValidKnightMove(fromRow, fromCol, toRow, toCol, errorsOff);
    } else if (piece->getType() == "bishop") {
        return isValidBishopMove(fromRow, fromCol, toRow, toCol, errorsOff);
    } else if (piece->getType() == "rook") {
        return isValidRookMove(fromRow, fromCol, toRow, toCol, errorsOff);
    } else if (piece->getType() == "queen") {
        return isValidQueenMove(fromRow, fromCol, toRow, toCol, errorsOff);
    } else if (piece->getType() == "king") {
        return isValidKingMove(fromRow, fromCol, toRow, toCol, errorsOff, checkCastling);
    }

    return false;
}

bool ChessBoard::isValidPawnMove(int fromRow, int fromCol, int toRow, int toCol, ChessPiece* piece, bool errorsOff) {
    bool isWhite = piece->getPlayer() == "white";
    int direction = (isWhite) ? 1 : -1; // White pawns move up, black pawns move down
    int startRow = (isWhite) ? 1 : 6; // Starting row for white and black pawns

    // Move forward by one square
    if (fromCol == toCol && toRow == fromRow + direction && board[toRow][toCol] == nullptr) {
        return true;
    }

    // Move forward by two squares from the starting position
    if (fromCol == toCol && fromRow == startRow && toRow == fromRow + 2 * direction && board[toRow][toCol] == nullptr && board[fromRow + direction][toCol] == nullptr) {
        return true;
    }

    // Capture diagonally
    if (abs(fromCol - toCol) == 1 && toRow == fromRow + direction && board[toRow][toCol] != nullptr && board[toRow][toCol]->getPlayer() != piece->getPlayer()) {
        return true;
    }

    return false;
}

bool ChessBoard::isValidKnightMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff) {
    if (board[toRow][toCol] != nullptr && board[toRow][toCol]->getPlayer() == board[fromRow][fromCol]->getPlayer()) {
        if (!errorsOff) cerr << "Destination square occupied by friendly piece!" << endl;
        return false;
    }
    return (abs(fromRow - toRow) == 2 && abs(fromCol - toCol) == 1) ||
            (abs(fromRow - toRow) == 1 && abs(fromCol - toCol) == 2);
}

bool ChessBoard::isValidBishopMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff) {
    if (abs(fromRow - toRow) != abs(fromCol - toCol)) {
        if (!errorsOff) cerr << "Bishop can only move diagonally!" << endl;
        return false;
    }

    int rowDirection = (toRow > fromRow) ? 1 : -1;
    int colDirection = (toCol > fromCol) ? 1 : -1;

    // Check each square along the diagonal path for obstacles
    int row = fromRow + rowDirection;
    int col = fromCol + colDirection;
    while (row != toRow && col != toCol) {
        if (board[row][col] != nullptr) {
            if (!errorsOff) cerr << "Piece blocking the bishop's path!" << endl;
            return false;
        }
        row += rowDirection;
        col += colDirection;
    }

    // Check if the destination square is occupied by a piece of the same player
    if (board[toRow][toCol] != nullptr && board[toRow][toCol]->getPlayer() == board[fromRow][fromCol]->getPlayer()) {
        if (!errorsOff) cerr << "Destination square occupied by friendly piece!" << endl;
        return false;
    }

    return true;
}

bool ChessBoard::isValidRookMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff) {
    if (fromRow != toRow && fromCol != toCol) {
        if (!errorsOff) cerr << "Rook can only move in a straight line!" << endl;
        return false;
    }

    int rowDirection = (toRow > fromRow) ? 1 : (toRow < fromRow) ? -1 : 0;
    int colDirection = (toCol > fromCol) ? 1 : (toCol < fromCol) ? -1 : 0;

    // Check each square along the path for obstacles
    int row = fromRow + rowDirection;
    int col = fromCol + colDirection;
    while (row != toRow || col != toCol) {
        if (board[row][col] != nullptr) {
            if (!errorsOff) cerr << "Piece blocking path: " << board[row][col]->getType() << " at " << row << ", " << col << endl;
            return false;
        }
        row += rowDirection;
        col += colDirection;
    }

    // Check if the destination square is occupied by a piece of the same player
    if (board[toRow][toCol] != nullptr && board[toRow][toCol]->getPlayer() == board[fromRow][fromCol]->getPlayer()) {
        if (!errorsOff) cerr << "Destination square occupied by friendly piece!" << endl;
        return false;
    }

    return true;
}

bool ChessBoard::isValidQueenMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff) {
    return isValidRookMove(fromRow, fromCol, toRow, toCol, true) ||
            isValidBishopMove(fromRow, fromCol, toRow, toCol, true);
}

bool ChessBoard::isValidKingMove(int fromRow, int fromCol, int toRow, int toCol, bool errorsOff, bool checkCastling) {
    if (board[toRow][toCol] != nullptr && board[toRow][toCol]->getPlayer() == board[fromRow][fromCol]->getPlayer()) {
        if (!errorsOff) cerr << "Destination square occupied by friendly piece!" << endl;
        return false;
    }
    bool valid = abs(fromRow - toRow) <= 1 && abs(fromCol - toCol) <= 1;

    // Check for castling
    if (checkCastling && !((board[fromRow][fromCol]->getPlayer() == "white") ? whiteInCheck : blackInCheck) && !board[fromRow][fromCol]->getHasMoved() && ((fromRow == 0 && fromCol == 4) || (fromRow == 7 && fromCol == 4))) {
        // Kingside castling
        if ((toRow == 0 && toCol == 6 && board[0][5] == nullptr && board[0][6] == nullptr && board[0][7] != nullptr && board[0][7]->getType() == "rook" && !board[0][7]->getHasMoved() &&
             !squareUnderAttackBy("e1", "black") && !squareUnderAttackBy("f1", "black") && !squareUnderAttackBy("g1", "black")) ||
            (toRow == 7 && toCol == 6 && board[7][5] == nullptr && board[7][6] == nullptr && board[7][7] != nullptr && board[7][7]->getType() == "rook" && !board[7][7]->getHasMoved() &&
             !squareUnderAttackBy("e8", "white") && !squareUnderAttackBy("f8", "white") && !squareUnderAttackBy("g8", "white"))) {
            valid = true;
            if (!errorsOff) {
                movePiece((playerTurn == "white") ? "h1f1" : "h8f8");
                playerTurn = (playerTurn == "white") ? "black" : "white";
                FEN = decrementFullMoveCounter(FEN);
                castleSound.play();
            }
        }
        // Queenside castling
        if ((toRow == 0 && toCol == 2 && board[0][1] == nullptr && board[0][2] == nullptr && board[0][3] == nullptr && board[0][0] != nullptr && board[0][0]->getType() == "rook" && !board[0][0]->getHasMoved() &&
             !squareUnderAttackBy("e1", "black") && !squareUnderAttackBy("d1", "black") && !squareUnderAttackBy("c1", "black")) ||
            (toRow == 7 && toCol == 2 && board[7][1] == nullptr && board[7][2] == nullptr && board[7][3] == nullptr && board[7][0] != nullptr && board[7][0]->getType() == "rook" && !board[7][0]->getHasMoved() &&
             !squareUnderAttackBy("e8", "white") && !squareUnderAttackBy("d8", "white") && !squareUnderAttackBy("c8", "white"))) {
            valid = true;
            if (!errorsOff) {
                movePiece((playerTurn == "white") ? "a1d1" : "a8d8");
                playerTurn = (playerTurn == "white") ? "black" : "white";
                FEN = decrementFullMoveCounter(FEN);
                castleSound.play();
            }
        }
    }
    return valid;
}

bool ChessBoard::squareUnderAttackBy(string square, string player) {
    // Check if any opponent pieces can attack the square
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] != nullptr && board[i][j]->getPlayer() == player) {
                string move = board[i][j]->getBoardLocation() + square;
                if (validMove(move, true, false)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void ChessBoard::movePiece(const string& move) {
    pair<int, int> src = getPositionFromNotation(move.substr(0, 2));
    pair<int, int> dst = getPositionFromNotation(move.substr(2, 2));

    ChessPiece* piece = board[src.second][src.first];

    if (move.length() < 4 || move.length() > 5) {
        cerr << "Invalid move string: " << move << endl;
        return;
    } else if (!validMove(move.substr(0, 4))) {
        if (piece != nullptr) {
            cerr << "Invalid move by " << piece->getPlayer() << ": " << move << endl;
        } else {
            cerr << "Invalid move: " << move << endl;
        }
        illegalSound.play();
        return;
    }

    // Save the piece pointer that was at the destination
    ChessPiece* capturedPiece = board[dst.second][dst.first];

    // Move the piece to the destination position
    board[dst.second][dst.first] = piece;
    board[src.second][src.first] = nullptr;
    piece->setPosition(dst.first, dst.second, 0);

    bool playNormalMoveSound = true;
    bool whiteInCheck = inCheck("white");
    bool blackInCheck = inCheck("black");
    if (piece->getPlayer() == "white" && whiteInCheck) {
        // Revert white's move if it puts white in check
        board[dst.second][dst.first] = capturedPiece;
        board[src.second][src.first] = piece;
        piece->setPosition(src.first, src.second, 0);
        cerr << "White in check!" << endl;
        illegalSound.play();
        return;
    } else if (piece->getPlayer() == "black" && blackInCheck) {
        // Revert black's move if it puts black in check
        board[dst.second][dst.first] = capturedPiece;
        board[src.second][src.first] = piece;
        piece->setPosition(src.first, src.second, 0);
        cerr << "Black in check!" << endl;
        illegalSound.play();
        return;
    } else if (piece->getPlayer() == "white" && blackInCheck || piece->getPlayer() == "black" && whiteInCheck) {
        checkSound.play();
        playNormalMoveSound = false;
    }

    // If there was a piece at the destination, remove it from the pieces vector and delete it
    if (capturedPiece != nullptr) {
        captureSound.play();
        auto it = find(pieces.begin(), pieces.end(), capturedPiece);
        if (it != pieces.end()) {
            delete *it;
            pieces.erase(it);
        }
    }

    // Check for pawn promotion default to queen if unspecified
    if (piece->getType() == "pawn" && (dst.second == 7 || dst.second == 0)) {
        char promotionType = (move.length() == 5) ? move[4] : 'q';
        string promotionTypeStr;
        switch (promotionType) {
            case 'q': promotionTypeStr = "queen"; break;
            case 'r': promotionTypeStr = "rook"; break;
            case 'b': promotionTypeStr = "bishop"; break;
            case 'n': promotionTypeStr = "knight"; break;
            default: promotionTypeStr = "queen"; break;
        }
        piece->convertTo(promotionTypeStr);
        piece->setPosition(dst.first, dst.second, 0);
        FEN = appendMoveToFEN(FEN, move);
        FEN = changePieceAtLocation(FEN, move.substr(2, 2), promotionTypeStr, piece->getPlayer());
    } else {
        piece->setPosition(dst.first, dst.second, 0);
        piece->setHasMoved(true);
        FEN = appendMoveToFEN(FEN, move);
    }

    playerTurn = (playerTurn == "white") ? "black" : "white";
    if (playNormalMoveSound) moveSound.play();
}

bool ChessBoard::testMove(const string& move) {
    if (move.length() != 4) {
        cerr << "Invalid move string: " << move << endl;
        return false;
    } else if (!validMove(move, true)) {
        return false;
    }

    pair<int, int> src = getPositionFromNotation(move.substr(0, 2));
    pair<int, int> dst = getPositionFromNotation(move.substr(2, 2));

    ChessPiece* piece = board[src.second][src.first];

    // Save the piece pointer at the destination
    ChessPiece* capturedPiece = board[dst.second][dst.first];

    // Move the piece to the destination position
    board[dst.second][dst.first] = piece;
    board[src.second][src.first] = nullptr;

    bool check = inCheck(piece->getPlayer());

    // Revert the move
    board[dst.second][dst.first] = capturedPiece;
    board[src.second][src.first] = piece;

    return !check;
}

string ChessBoard::getSquareAtCenter() {
    // Calculate the intersection point of viewing direction with the board on the board plane
    glm::mat4 invProjView = glm::inverse(ProjectionMatrix * ViewMatrix);
    glm::vec4 ndcCenter = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
    glm::vec4 worldNearPoint = invProjView * ndcCenter;
    worldNearPoint /= worldNearPoint.w;
    ndcCenter.z = 1.0f;
    glm::vec4 worldFarPoint = invProjView * ndcCenter;
    worldFarPoint /= worldFarPoint.w;
    glm::vec3 rayDir = glm::vec3(worldFarPoint - worldNearPoint);
    float t = -worldNearPoint.y / rayDir.y;
    glm::vec3 intersectionPoint = glm::vec3(worldNearPoint) + t * rayDir;

    int x = 4 - round(4 * intersectionPoint.x / 1.2 + 0.5);
    int y = 4 + round(4 * intersectionPoint.z / 1.2 - 0.5);

    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return "";
    }
    return string(1, 'a' + x) + to_string(y + 1);
}

void ChessBoard::checkHoveredPieces() {
    hoveredPieceLocation = "";
    bool pieceHovered = false;
    for (auto& piece : pieces) {
        if (piece->getPlayer() != "white" && !overrideMode) continue;
        if (!pieceHovered && piece->getBoardLocation() == targetPointerLocation) {
            piece->setHovered(true);
            hoveredPieceLocation = piece->getBoardLocation();
            pieceHovered = true;
        } else {
            if (selectedPieceLocation != piece->getBoardLocation()) piece->setHovered(false);
        }
    }
}

void ChessBoard::userInteraction() {
    targetPointerLocation = getSquareAtCenter();
    checkHoveredPieces();
    
    // Check if the mouse button is pressed and set the hovered piece location to the selected piece location
    if (selectedPieceLocation != hoveredPieceLocation && hoveredPieceLocation != "" && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        selectedPieceLocation = hoveredPieceLocation;
        while (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {glfwPollEvents();}
    } else if (selectedPieceLocation == hoveredPieceLocation && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        selectedPieceLocation = "";
        while (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {glfwPollEvents();}
    } else if (targetPointerLocation != "" && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        movePiece(selectedPieceLocation + targetPointerLocation);
        selectedPieceLocation = "";
        while (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {glfwPollEvents();}
    }

    // Check if the game is over
    if (checkMatedTime == 0 && checkMated("black")) {
        checkMatedTime = glfwGetTime();
        cerr << "Checkmate! White wins!" << endl;
        checkmateSound.play();
    } else if (checkMatedTime == 0 && checkMated("white")) {
        checkMatedTime = glfwGetTime();
        cerr << "Checkmate! Black wins!" << endl;
        checkmateSound.play();
    } else if (checkMated("black") || checkMated("white")) {
        double elapsed = glfwGetTime() - checkMatedTime;
        float expectedDuration = 3.0f;
        if (!opponentProcessing && elapsed >= expectedDuration) {
            reset();
        }
    }
}

void ChessBoard::update() {
    if (!gameRunning) return;

    // Check if it is the opponent's turn and get the opponent's move
    if (!overrideMode && playerTurn == "black" && !opponentProcessing && !opponentMoveReceived) {
        opponentProcessing = true;
        thread opponentThread(&ChessBoard::getOpponentMove, this);
        opponentThread.detach();
    } else if (!overrideMode && playerTurn == "black" && !opponentProcessing && opponentMoveReceived) {
        // Execute the opponent's move
        movePiece(opponentMove);
        opponentMove = "";
        opponentMoveReceived = false;
    }
    userInteraction();
}

void ChessBoard::getOpponentMove() {
    auto start = chrono::high_resolution_clock::now();
    
    string move = stockfish.getMove(FEN);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    float expectedDuration = 1.0f;
    // Wait for at least 1 second before returning the move
    if (elapsed.count() < expectedDuration) {
        this_thread::sleep_for(chrono::duration<double>(expectedDuration - elapsed.count()));
    }

    if (move.empty()) {
        cout << "Game Over" << endl;
        opponentProcessing = false;
        return;
    }
    opponentMove = move;
    opponentProcessing = false;
    opponentMoveReceived = true;
}

void ChessBoard::generateBoardMesh() {
    float vertices[] = {
        // Positions       // Normals         // Texture Coords
        1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Bottom-left
        -1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // Bottom-right
        -1.0f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // Top-right
        1.0f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,  // Top-left
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &boardVAO);
    GLuint VBO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(boardVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    // Apply transformations
    modelMatrix = glm::translate(glm::mat4(1.0f), boardPosition);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.2f, 1.0f, 1.2f));
}

void ChessBoard::generateCheckerboardTexture() {
    int width = 512;
    int height = 512;
    int squareSize = width / 8;
    vector<unsigned char> data(width * height * 3);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int squareX = x / squareSize;
            int squareY = y / squareSize;
            unsigned char color = (squareX % 2 == squareY % 2) ? 0 : 255;

            data[(y * width + x) * 3 + 0] = color; // R
            data[(y * width + x) * 3 + 1] = color; // G
            data[(y * width + x) * 3 + 2] = color; // B
        }
    }

    glGenTextures(1, &boardTexture);
    glBindTexture(GL_TEXTURE_2D, boardTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void ChessBoard::reset() {
    gameRunning = false;
    playerTurn = "white";
    FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    hoveredPieceLocation = "";
    selectedPieceLocation = "";
    targetPointerLocation = "";
    checkMatedTime = 0;
    whiteInCheck = false;
    blackInCheck = false;

    camera.setPositionAndOrientation(glm::vec3(0.0f, 3.0f, -2.5f), glm::vec3(0.0f, 0.0f, 0.0f), 90.0f, -50.0f);

    for (auto piece : pieces) {
        delete piece;
        piece = nullptr;
    }
    pieces.clear();

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j] = nullptr;
        }
    }

    // Initialize the white pieces
    addPiece(new ChessPiece(0, 0, 0, "rook", "white"), 0, 0);
    addPiece(new ChessPiece(1, 0, 0, "knight", "white"), 1, 0);
    addPiece(new ChessPiece(2, 0, 0, "bishop", "white"), 2, 0);
    addPiece(new ChessPiece(3, 0, 0, "queen", "white"), 3, 0);
    addPiece(new ChessPiece(4, 0, 0, "king", "white"), 4, 0);
    addPiece(new ChessPiece(5, 0, 0, "bishop", "white"), 5, 0);
    addPiece(new ChessPiece(6, 0, 0, "knight", "white"), 6, 0);
    addPiece(new ChessPiece(7, 0, 0, "rook", "white"), 7, 0);

    for (int i = 0; i < 8; ++i) {
        addPiece(new ChessPiece(i, 1, 0, "pawn", "white"), i, 1);
    }

    // Initialize the black pieces
    addPiece(new ChessPiece(0, 7, 0, "rook", "black"), 0, 7);
    addPiece(new ChessPiece(1, 7, 0, "knight", "black"), 1, 7);
    addPiece(new ChessPiece(2, 7, 0, "bishop", "black"), 2, 7);
    addPiece(new ChessPiece(3, 7, 0, "queen", "black"), 3, 7);
    addPiece(new ChessPiece(4, 7, 0, "king", "black"), 4, 7);
    addPiece(new ChessPiece(5, 7, 0, "bishop", "black"), 5, 7);
    addPiece(new ChessPiece(6, 7, 0, "knight", "black"), 6, 7);
    addPiece(new ChessPiece(7, 7, 0, "rook", "black"), 7, 7);

    for (int i = 0; i < 8; ++i) {
        addPiece(new ChessPiece(i, 6, 0, "pawn", "black"), i, 6);
    }
}
