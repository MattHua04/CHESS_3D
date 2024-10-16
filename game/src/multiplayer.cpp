#include "multiplayer.h"

int clientSocket; // Socket for the client
queue<string> opponentMoves; // Queue of opponent moves
mutex opponentMovesMutex; // Mutex for the opponent moves queue
thread listenMultiplayerThread; // Thread to listen for opponent moves
atomic<bool> listenForMove; // Flag to indicate if the client should listen for opponent moves

string lookForOpponent() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Failed to create clientSocket." << endl;
        multiplayer = false;
        return "white";
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_ADDRESS, &serverAddress.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Failed to connect to the server: " << strerror(errno) << endl;
        multiplayer = false;
        return "white";
    }
    cout << "Connected to server." << endl;

    // Get color assignment from server
    string color;
    uint32_t color_length;

    ssize_t bytesReceived = recv(clientSocket, &color_length, sizeof(color_length), 0);
    if (bytesReceived <= 0) {
        cerr << "Failed to receive color length from server." << endl;
        close(clientSocket);
        multiplayer = false;
        return "white";
    }

    color_length = ntohl(color_length);

    char buffer[1024];
    bytesReceived = recv(clientSocket, buffer, color_length, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        color = string(buffer);
    } else if (bytesReceived == 0) {
        cerr << "Server closed the connection before sending color assignment." << endl;
        close(clientSocket);
        multiplayer = false;
        return "white";
    } else {
        cerr << "Failed to receive color assignment from server: " << strerror(errno) << endl;
        close(clientSocket);
        multiplayer = false;
        return "white";
    }

    if (color == "white") {
        cout << "You are playing as White." << endl;
    } else if (color == "black") {
        cout << "You are playing as Black." << endl;
    } else {
        cerr << "Invalid color received from server: " << color << endl;
        multiplayer = false;
        color = "white";
        close(clientSocket);
    }

    listenForMove = true;
    listenMultiplayerThread = thread(listenForMultiplayerMove);
    listenMultiplayerThread.detach();
    return color;
}

void listenForMultiplayerMove() {
    char buffer[1024];
    while (true) {
        if (!listenForMove) return;

        uint32_t move_length_network;
        ssize_t bytesReceived = recv(clientSocket, &move_length_network, sizeof(move_length_network), 0);
        if (bytesReceived <= 0) {
            cout << "Connection closed by server or error receiving length." << endl;
            close(clientSocket);
            multiplayer = false;
            break;
        }

        uint32_t move_length = ntohl(move_length_network);

        bytesReceived = recv(clientSocket, buffer, move_length, 0);
        if (bytesReceived <= 0) {
            cout << "Connection closed by server or error receiving move." << endl;
            close(clientSocket);
            multiplayer = false;
            break;
        }

        buffer[bytesReceived] = '\0';

        if (string(buffer) == "reset") {
            resetBoard = true;
            continue;
        }

        {
            lock_guard<mutex> lock(opponentMovesMutex);
            opponentMoves.push(string(buffer));
        }

        cout << "Received move: " << buffer << endl;
    }
}

void sendMove(const string& move) {
    cout << "Sending move: " << move << endl;
    uint32_t move_length = htonl(move.size());

    if (send(clientSocket, &move_length, sizeof(move_length), 0) == -1) {
        cerr << "Failed to send move length to server: " << strerror(errno) << endl;
        close(clientSocket);
        multiplayer = false;
        return;
    }

    if (send(clientSocket, move.c_str(), move.size(), 0) == -1) {
            cerr << "Failed to send move to server: " << strerror(errno) << endl;
            close(clientSocket);
            multiplayer = false;
    }
}

string getMultiplayerMove() {
    string move;
    while (move.empty()) {
        {
            lock_guard<mutex> lock(opponentMovesMutex);
            while (!opponentMoves.empty()) {
                move = opponentMoves.front();
                opponentMoves.pop();
            }
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    return move;
}

void cleanupMultiplayer() {
    listenForMove = false;
    close(clientSocket);
}