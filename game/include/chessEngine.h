#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include "globals.h"

using namespace std;

/**
 * @class Stockfish
 * @brief Represents the Stockfish chess engine.
 */
class Stockfish {
public:
    /**
     * @brief Default constructor.
     */
    Stockfish(bool remote=false) : depth(0), stockfishProcess(nullptr), remoteProcessing(remote) {};

    /**
     * @brief Destructor.
     */
    ~Stockfish();

    /**
     * @brief Initializes the Stockfish engine.
     */
    void init();

    /**
     * @brief Sets the depth of the Stockfish engine.
     * @param depth The depth to set.
     */
    void setDepth(int depth) { this->depth = depth; }

    /**
     * @brief Sets the difficulty of the Stockfish engine.
     * @param difficulty The difficulty to set.
     */
    void setDifficulty(int difficulty);

    /**
     * @brief Sets whether to process moves remotely with API calls.
     * @param remote Whether to process moves remotely.
     */
    void setRemoteProcessing(bool remote) { remoteProcessing = remote; }

    /**
     * @brief Gets the best move for a given board position.
     * @param boardPosition The board position to get the best move for.
     * @return The best move.
     */
    string getMove(const string& boardPosition);

    /**
     * @brief Gets the best move for a given board position using local processing.
     * @param boardPosition The board position to get the best move for.
     * @return The best move.
     */
    string getMoveLocal(const string& boardPosition);

    /**
     * @brief Gets the best move for a given board position using remote processing.
     * @param boardPosition The board position to get the best move for.
     * @return The best move.
     */
    string getMoveRemote(const string& boardPosition);

private:
    bool remoteProcessing; // Whether to process moves remotely
    int depth; // The depth of the Stockfish engine
    int difficulty; // The difficulty of the Stockfish engine
    FILE* stockfishProcess; // The Stockfish process
    void sendCommand(const string& command); // Sends a command to the Stockfish engine
    string readResponse(); // Reads the response from the Stockfish engine
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output); // Callback function for writing response
    string sendGetRequest(const string& url); // Sends a GET request to an API
    string parseMove(const string& response); // Parses the best move from the response
};

extern Stockfish stockfish;

#endif