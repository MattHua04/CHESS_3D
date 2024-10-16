#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include "globals.h"

using namespace std;

/**
 * @brief Looks for an opponent to play against.
 */
string lookForOpponent();

/**
 * @brief Get the opponent's current move.
 */
string getMultiplayerMove();

/**
 * @brief Constantly listen for the opponent's move.
 */
void listenForMultiplayerMove();

/**
 * @brief Sends a move to the opponent.
 * @param move The move to send.
 */
void sendMove(const string& move);

/**
 * @brief Cleans up the multiplayer game.
 */
void cleanupMultiplayer();

#endif