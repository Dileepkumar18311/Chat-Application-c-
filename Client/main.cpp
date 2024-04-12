#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <string>

using namespace std;
#pragma comment(lib,"ws2_32.lib")

/*
1. Initialize Winsock
2. Create socket
3. Connect to the server
4. Send/receive messages
5. Close the socket
*/

bool InitializeWinsock() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

void SendMessageToServer(SOCKET clientSocket) {
    cout << "Enter your chat name: " << endl;
    string name;
    getline(cin, name);
    string message;

    while (true) {
        getline(cin, message);
        string msg = name + " : " + message;
        int bytesSent = send(clientSocket, msg.c_str(), msg.length(), 0);
        if (bytesSent == SOCKET_ERROR) {
            cout << "Error sending message" << endl;
            break;
        }
        if (message == "quit") {
            cout << "Stopping the application" << endl;
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
}

void ReceiveMessageFromServer(SOCKET clientSocket) {
    char buffer[4096];
    int receivedLength;
    string message = "";

    while (true) {
        receivedLength = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (receivedLength <= 0) {
            cout << "Disconnected from the server" << endl;
            break;
        }
        else {
            message = string(buffer, receivedLength);
            cout << message << endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
}

int main() {
    if (!InitializeWinsock()) {
        cout << "Winsock initialization failed" << endl;
        return 1;
    }

    SOCKET clientSocket;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Invalid socket created" << endl;
        return 1;
    }

    int port = 12345;
    string serverAddress = "127.0.0.1";
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverAddress.c_str(), &(serverAddr.sin_addr));

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Failed to connect to the server" << endl;
        cout << ": " << WSAGetLastError();
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    cout << "Successfully connected to the server" << endl;

    thread senderThread(SendMessageToServer, clientSocket);
    thread receiverThread(ReceiveMessageFromServer, clientSocket);

    senderThread.join();
    receiverThread.join();

    return 0;
}
