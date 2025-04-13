#include "chat.h"
#include <iostream>
#include <cstring>

Chat::Chat() {
    Init();
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: Failed to initialize winsock." << std::endl;
    }
#endif
}

Chat::~Chat() {
    Close();
#ifdef _WIN32
    WSACleanup();
#endif
}

void Chat::Init() {
    isRunning = false;
    des.SetKey(KEY);
}

void Chat::Connect() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Failed to connect to server." << std::endl;
        return;
    }
    isRunning = true;

    receiveThread = std::thread(&Chat::ReceiveThread, this);
}

void Chat::Send() {
    std::cin.getline(message, MAX_MESSAGE_LENGTH);
    char* cipherText = nullptr;
    int cipherTextLength = -1;
    des.Encrypt(message, strlen(message), cipherText, cipherTextLength);

    if (send(clientSocket, cipherText, cipherTextLength, 0) < 0) {
        std::cerr << "Error: Failed to send message." << std::endl;
    }
    delete[] cipherText;
}

void Chat::ReceiveThread() {
    const char* info = isServer ? "Client" : "Server";
    char* plainText = nullptr;
    int plainTextLength = -1;

    while (isRunning) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t len = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (len <= 0) {
            std::cerr << "Error: Failed to receive message or connection closed." << std::endl;
            isRunning = false;
            break;
        }
        des.Decrypt(buffer, len, plainText, plainTextLength);
        std::cout << info << ": " << plainText << std::endl;
        delete[] plainText;

        if(0==memcmp("quit",buffer,4)){
            std::cout << "Connection closed by " << info << "." << std::endl;
            isRunning = false;
            break;
        }

    }
}

void Chat::Close() {
    isRunning = false;
    if (serverSocket >= 0) {
        close(serverSocket);
    }
    if (clientSocket >= 0) {
        close(clientSocket);
    }
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
}

void Chat::RunServer() {
    isServer = true;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Failed to bind." << std::endl;
        return;
    }

    if (listen(serverSocket, 1) < 0) {
        std::cerr << "Error: Failed to listen." << std::endl;
        return;
    }

    sockaddr_in clientAddr{};
    socklen_t clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to accept." << std::endl;
        return;
    }

    isRunning = true;
    receiveThread = std::thread(&Chat::ReceiveThread, this);

    while (isRunning) {
        Send();
    }

    Close();
}

void Chat::RunClient() {
    isServer = false;
    Connect();
    while (isRunning) {
        Send();
    }
    Close();
}