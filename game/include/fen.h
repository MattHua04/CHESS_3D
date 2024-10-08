#ifndef FEN_H
#define FEN_H

#include "globals.h"

using namespace std;

string appendMoveToFEN(string fen, const string& move);

string changePieceAtLocation(string fen, string location, string newType, string player);

void printFEN(const string& fen);

#endif