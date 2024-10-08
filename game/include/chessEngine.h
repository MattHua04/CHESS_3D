#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include "globals.h"

using namespace std;

class Stockfish {
public:
    Stockfish(bool remote=false) : depth(0), stockfishProcess(nullptr), remoteProcessing(remote) {};
    ~Stockfish();
    void init();
    void setDepth(int depth) { this->depth = depth; }
    void setDifficulty(int difficulty);
    void setRemoteProcessing(bool remote) { remoteProcessing = remote; }
    string getMove(const string& boardPosition);
    string getMoveLocal(const string& boardPosition);
    string getMoveRemote(const string& boardPosition);

private:
    bool remoteProcessing;
    int depth;
    int difficulty;
    FILE* stockfishProcess;
    void sendCommand(const string& command);
    string readResponse();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output);
    string sendGetRequest(const string& url);
    string parseMove(const string& response);
};

extern Stockfish stockfish;

#endif