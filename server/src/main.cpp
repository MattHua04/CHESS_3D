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

using namespace std;

const int PORT = 12345;
mutex queue_mutex;
queue<int> client_queue;

void handlePlayer(int sender, int receiver) {
    char buffer[1024];
    while (true) {
        // Receive message length from source player
        uint32_t message_length_network;
        ssize_t bytes_received = recv(sender, &message_length_network, sizeof(message_length_network), 0);
        if (bytes_received <= 0) break;

        uint32_t message_length = ntohl(message_length_network);

        // Receive the actual message from source player
        bytes_received = recv(sender, buffer, message_length, 0);
        if (bytes_received <= 0) break;
        buffer[bytes_received] = '\0';

        // Forward the message length to destination player
        send(receiver, &message_length_network, sizeof(message_length_network), 0);
        // Forward the actual message to destination player
        send(receiver, buffer, message_length, 0);
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

void pairAvailablePlayers(int server_socket) {
    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len);

        if (client_socket < 0) {
            cerr << "Failed to accept player." << endl;
            continue;
        }

        cout << "Player connected." << endl;

        // Add players to the queue
        {
            lock_guard<mutex> lock(queue_mutex);
            client_queue.push(client_socket);
        }

        // Pair two players together
        {
            lock_guard<mutex> lock(queue_mutex);
            if (client_queue.size() >= 2) {
                int player1 = client_queue.front();
                client_queue.pop();
                int player2 = client_queue.front();
                client_queue.pop();

                thread(startGame, player1, player2).detach();
            }
        }
    }
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "Failed to create socket." << endl;
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (::bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 5) < 0) {
        cerr << "Failed to listen on socket." << endl;
        return 1;
    }

    cout << "Server is listening on port " << PORT << endl;
    pairAvailablePlayers(server_socket);

    close(server_socket);
    return 0;
}