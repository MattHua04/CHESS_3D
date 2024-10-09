#include "fen.h"

using namespace std;

string appendMoveToFEN(string fen, const string& move) {
    if (move.empty()) {
        return fen;
    }

    // Split the FEN string into its components
    vector<string> fenParts;
    stringstream ss(fen);
    string part;
    while (getline(ss, part, ' ')) {
        fenParts.push_back(part);
    }

    // Update the board position based on the move
    string board = fenParts[0];
    string from = move.substr(0, 2);
    string to = move.substr(2, 2);

    // Convert board to a 2D array
    vector<vector<char>> boardArray(8, vector<char>(8, ' '));
    int index = 0;
    for (char c : board) {
        if (isdigit(c)) {
            index += c - '0';
        } else if (c == '/') {
            continue;
        } else {
            boardArray[index / 8][index % 8] = c;
            index++;
        }
    }

    // Convert FROM and TO positions to board indices
    int fromRow = 8 - (from[1] - '0');
    int fromCol = from[0] - 'a';
    int toRow = 8 - (to[1] - '0');
    int toCol = to[0] - 'a';

    // Update the halfmove clock
    if (boardArray[toRow][toCol] == 'P' || boardArray[toRow][toCol] == 'p' || boardArray[toRow][toCol] != ' ') {
        fenParts[4] = "0";
    } else {
        fenParts[4] = to_string(stoi(fenParts[4]) + 1);
    }

    // Update the fullmove number
    if (fenParts[1] == "b") {
        fenParts[5] = to_string(stoi(fenParts[5]) + 1);
    }

    // Make the move on the board
    boardArray[toRow][toCol] = boardArray[fromRow][fromCol];
    boardArray[fromRow][fromCol] = ' ';

    // Handle castling moves
    if (boardArray[toRow][toCol] == 'K' && from == "e1") {
        if (to == "g1") { // White kingside castling
            boardArray[7][5] = 'R';
            boardArray[7][7] = ' ';
        } else if (to == "c1") { // White queenside castling
            boardArray[7][3] = 'R';
            boardArray[7][0] = ' ';
        }
    } else if (boardArray[toRow][toCol] == 'k' && from == "e8") {
        if (to == "g8") { // Black kingside castling
            boardArray[0][5] = 'r';
            boardArray[0][7] = ' ';
        } else if (to == "c8") { // Black queenside castling
            boardArray[0][3] = 'r';
            boardArray[0][0] = ' ';
        }
    }

    // Convert the board back to FEN format
    string newBoard;
    for (int i = 0; i < 8; i++) {
        int emptyCount = 0;
        for (int j = 0; j < 8; j++) {
            if (boardArray[i][j] == ' ') {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    newBoard += to_string(emptyCount);
                    emptyCount = 0;
                }
                newBoard += boardArray[i][j];
            }
        }
        if (emptyCount > 0) {
            newBoard += to_string(emptyCount);
        }
        if (i < 7) {
            newBoard += '/';
        }
    }

    // Update the FEN parts with the new board position
    fenParts[0] = newBoard;

    // Update the active color (who has moved)
    fenParts[1] = (fenParts[1] == "w") ? "b" : "w";

    // Update castling availability
    string castling = fenParts[2];
    if (from == "e1" || to == "e1") {
        castling.erase(remove(castling.begin(), castling.end(), 'K'), castling.end());
        castling.erase(remove(castling.begin(), castling.end(), 'Q'), castling.end());
    }
    if (from == "h1" || to == "h1") {
        castling.erase(remove(castling.begin(), castling.end(), 'K'), castling.end());
    }
    if (from == "a1" || to == "a1") {
        castling.erase(remove(castling.begin(), castling.end(), 'Q'), castling.end());
    }
    if (from == "e8" || to == "e8") {
        castling.erase(remove(castling.begin(), castling.end(), 'k'), castling.end());
        castling.erase(remove(castling.begin(), castling.end(), 'q'), castling.end());
    }
    if (from == "h8" || to == "h8") {
        castling.erase(remove(castling.begin(), castling.end(), 'k'), castling.end());
    }
    if (from == "a8" || to == "a8") {
        castling.erase(remove(castling.begin(), castling.end(), 'q'), castling.end());
    }
    fenParts[2] = castling.empty() ? "-" : castling;

    // Reconstruct the FEN string
    string newFEN;
    for (const string& part : fenParts) {
        newFEN += part + " ";
    }
    newFEN.pop_back();

    return newFEN;
}

string changePieceAtLocation(string fen, string location, string newType, string player) {
    // Split the FEN string into its components
    vector<string> fenParts;
    stringstream ss(fen);
    string part;
    while (getline(ss, part, ' ')) {
        fenParts.push_back(part);
    }

    // Convert board to a 2D array
    string board = fenParts[0];
    vector<vector<char>> boardArray(8, vector<char>(8, ' '));
    int index = 0;
    for (char c : board) {
        if (isdigit(c)) {
            index += c - '0';
        } else if (c == '/') {
            continue;
        } else {
            boardArray[index / 8][index % 8] = c;
            index++;
        }
    }

    // Convert location to board indices
    int row = 8 - (location[1] - '0');
    int col = location[0] - 'a';

    // Change the piece at the specified location
    char piece = (player == "white") ? toupper(newType[0]) : tolower(newType[0]);
    boardArray[row][col] = piece;

    // Convert the board back to FEN format
    string newBoard;
    for (int i = 0; i < 8; i++) {
        int emptyCount = 0;
        for (int j = 0; j < 8; j++) {
            if (boardArray[i][j] == ' ') {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    newBoard += to_string(emptyCount);
                    emptyCount = 0;
                }
                newBoard += boardArray[i][j];
            }
        }
        if (emptyCount > 0) {
            newBoard += to_string(emptyCount);
        }
        if (i < 7) {
            newBoard += '/';
        }
    }

    // Update the FEN parts with the new board position
    fenParts[0] = newBoard;

    // Reconstruct the FEN string
    string newFEN;
    for (const string& part : fenParts) {
        newFEN += part + " ";
    }
    newFEN.pop_back();

    return newFEN;
}

string decrementFullMoveCounter(string fen) {
    vector<string> fenParts;
    stringstream ss(fen);
    string part;
    while (getline(ss, part, ' ')) {
        fenParts.push_back(part);
    }
    
    // Decrement the fullmove counter
    if (!fenParts.empty()) {
        stringstream ss(fenParts[5]);
        int fullmoveCounter;
        ss >> fullmoveCounter;
        if (fullmoveCounter > 1) {
            fullmoveCounter--;
        }
        fenParts[5] = to_string(fullmoveCounter);
    }

    // Switch the player who should move
    if (!fenParts.empty()) {
        fenParts[1] = (fenParts[1] == "w") ? "b" : "w";
    }

    string newFEN;
    for (const string& part : fenParts) {
        newFEN += part + " ";
    }
    newFEN.pop_back();

    return newFEN;
}

void printFEN(const string& fen) {
    // Split the FEN string into parts
    string boardPart = fen.substr(0, fen.find(' '));
    
    // Create an 8x8 board representation
    char board[8][8];
    
    // Initialize the board with empty squares
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            board[r][c] = '.'; // Use '.' for empty squares
        }
    }

    int row = 0, col = 0;

    for (char c : boardPart) {
        if (isdigit(c)) { // If the character is a digit
            int emptySquares = c - '0'; // Convert char to int
            for (int i = 0; i < emptySquares; ++i) {
                if (col < 8) { // Ensure we do not overflow the board
                    board[row][col++] = '.'; // Use '.' for empty squares
                }
            }
        } else if (c == '/') { // Move to the next row
            row++;
            col = 0; // Reset column index
        } else { // Place the piece on the board
            if (col < 8) { // Ensure we do not overflow the board
                board[row][col++] = c;
            }
        }
    }

    // Print the board
    for (int r = 7; r >= 0; --r) {
        for (int c = 0; c < 8; ++c) {
            cout << board[r][c] << ' ';
        }
        cout << endl;
    }
}