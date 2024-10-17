#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>
#include <queue>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <csignal>

using namespace std;

int serverSocket;
const int PORT = 12345;
mutex queueMutex;
queue<int> clientQueue;

void handlePlayer(int sender, int receiver) {
    char buffer[1024];
    while (true) {
        // Receive message length from source player
        uint32_t messageLengthNetwork;
        ssize_t bytesRecieved = recv(sender, &messageLengthNetwork, sizeof(messageLengthNetwork), 0);
        if (bytesRecieved <= 0) break;

        uint32_t messageLength = ntohl(messageLengthNetwork);

        // Receive the actual message from source player
        bytesRecieved = recv(sender, buffer, messageLength, 0);
        if (bytesRecieved <= 0) break;
        buffer[bytesRecieved] = '\0';

        // Forward the message length to destination player
        send(receiver, &messageLengthNetwork, sizeof(messageLengthNetwork), 0);
        // Forward the actual message to destination player
        send(receiver, buffer, messageLength, 0);
    }

    close(sender);
}

void startGame(int client1, int client2) {
    srand(time(0));
    int color1 = rand() % 2;
    int color2 = 1 - color1;

    string color1_msg = (color1 == 0) ? "white" : "black";
    string color2_msg = (color2 == 0) ? "white" : "black";

    // Send the length of the message followed by the message
    uint32_t color1_length = htonl(color1_msg.size());
    send(client1, &color1_length, sizeof(color1_length), 0);
    send(client1, color1_msg.c_str(), color1_msg.size(), 0);

    // Send the length of the message followed by the message
    uint32_t color2_length = htonl(color2_msg.size());
    send(client2, &color2_length, sizeof(color2_length), 0);
    send(client2, color2_msg.c_str(), color2_msg.size(), 0);

    // Threads for handling communication between each player
    thread client1Thread(handlePlayer, client1, client2);
    thread client2Thread(handlePlayer, client2, client1);

    client1Thread.join();
    client2Thread.join();

    close(client1);
    close(client2);
    cout << "Game ended." << endl;
}

bool isPlayerConnected(int playerSocket) {
    char buffer;
    // Check if the player is connected
    int result = recv(playerSocket, &buffer, 1, MSG_DONTWAIT);
    
    if (result < 0) {
        if (errno == ECONNRESET) {
            return false;
        }
    } else if (result == 0) {
        // Connection was closed by the player
        return false;
    }
    
    return true;
}

void pairAvailablePlayers(int serverSocket) {
    while (true) {
        struct sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);

        if (clientSocket < 0) {
            cerr << "Failed to accept player." << endl;
            continue;
        }

        cout << "Player connected." << endl;

        // Add players to the queue
        {
            lock_guard<mutex> lock(queueMutex);
            clientQueue.push(clientSocket);
        }

        // Pair two players together
        {
            lock_guard<mutex> lock(queueMutex);
            while (clientQueue.size() >= 2) {
                int player1 = clientQueue.front();
                clientQueue.pop();
                int player2 = clientQueue.front();
                clientQueue.pop();

                // Check if both players are still connected
                if (isPlayerConnected(player1) && isPlayerConnected(player2)) {
                    thread(startGame, player1, player2).detach();
                } else {
                    // If one player is disconnected, keep the connected player and check again
                    if (isPlayerConnected(player1)) {
                        clientQueue.push(player1);
                    } else {
                        cout << "Player " << player1 << " has disconnected." << endl;
                        close(player1); // Close the socket for the disconnected player
                    }

                    if (isPlayerConnected(player2)) {
                        clientQueue.push(player2);
                    } else {
                        cout << "Player " << player2 << " has disconnected." << endl;
                        close(player2); // Close the socket for the disconnected player
                    }
                }
            }
        }
    }
}

void signalHandler(int signum) {
    cout << "\nInterrupt signal (" << signum << ") received. Closing server socket." << endl;
    close(serverSocket);
    exit(signum);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Failed to create socket." << endl;
        return 1;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (::bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 5) < 0) {
        cerr << "Failed to listen on socket." << endl;
        return 1;
    }

    cout << "Server is listening on port " << PORT << endl;
    pairAvailablePlayers(serverSocket);

    close(serverSocket);
    return 0;
}