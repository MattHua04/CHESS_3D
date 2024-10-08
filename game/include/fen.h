#ifndef FEN_H
#define FEN_H

#include "globals.h"

using namespace std;

/**
 * @brief Appends a move to the FEN string.
 * @param fen The FEN string.
 * @param move The move to append.
 * @return The updated FEN string.
 */
string appendMoveToFEN(string fen, const string& move);

/**
 * @brief Changes the piece at a location in the FEN string.
 * @param fen The FEN string.
 * @param location The location to change.
 * @param newType The new type of the piece.
 * @param player The player that owns the piece.
 * @return The updated FEN string.
 */
string changePieceAtLocation(string fen, string location, string newType, string player);

/**
 * @brief Prints the FEN string to the console.
 * @param fen The FEN string to print.
 */
void printFEN(const string& fen);

#endif