#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>

using namespace std;
#pragma comment(lib,"ws2_32.lib")

/*
1. Initialize Winsock library
2. Create the socket
3. Obtain IP address and port for the socket
4. Listen on the socket
5. Accept incoming connections
6. Receive and send data
7. Close the socket
*/

bool InitializeWinsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void HandleClientCommunication(SOCKET clientSocket, vector<SOCKET>& connectedClients) {
    // Send/receive data to/from the client

    cout << "Client connected" << endl;

    char buffer[4096];

    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            cout << "Client disconnected" << endl;
            break;
        }

        string message(buffer, bytesReceived);
        cout << "Message from client: " << message << endl;

        for (auto client : connectedClients) {
            if (client != clientSocket) {
                send(client, message.c_str(), message.length(), 0);
            }
        }
    }

    auto it = find(connectedClients.begin(), connectedClients.end(), clientSocket);
    if (it != connectedClients.end()) {
        connectedClients.erase(it);
    }

    closesocket(clientSocket);
}

int main() {
    if (!InitializeWinsock()) {
        cout << "Winsock initialization failed" << endl;
        return 1;
    }

    SOCKET listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket == INVALID_SOCKET) {
        cout << "Failed to create socket" << endl;
        return 1;
    }

    // Create address structure
    int port = 12345;
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    // Convert the IP address ("0.0.0.0") and store it in the address structure in binary form
    if (InetPton(AF_INET, _T("0.0.0.0"), &serverAddress.sin_addr) != 1) {
        cout << "Failed to create address structure" << endl;
        closesocket(listeningSocket);
        WSACleanup();
        return 1;
    }

    // Bind the socket
    if (bind(listeningSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Binding failed" << endl;
        closesocket(listeningSocket);
        WSACleanup();
        return 1;
    }

    // Start listening
    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen failed" << endl;
        closesocket(listeningSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server has started listening on port: " << port << endl;

    vector<SOCKET> connectedClients;

    while (true) {
        // Accept incoming connection
        SOCKET clientSocket = accept(listeningSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cout << "Invalid client socket" << endl;
        }

        connectedClients.push_back(clientSocket);
        thread clientThread(HandleClientCommunication, clientSocket, std::ref(connectedClients));
        clientThread.detach();
    }

    closesocket(listeningSocket);
    WSACleanup();
    return 0;
}
