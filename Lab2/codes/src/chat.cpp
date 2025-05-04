// Chat
#include "chat.h"
#include <iostream>
#include <cstring>
#include <fcntl.h>      // 用于 fcntl
#include <sys/select.h> // 用于 select

Chat::Chat() {
    Init();
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: Failed to initialize winsock." << std::endl;
        return;
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
    serverSocket = -1;
    clientSocket = -1;
    serverIp = DEFAULT_SERVER_IP;
    serverPort = DEFAULT_SERVER_PORT;
    isRunning = false;
    exited = false;
}

// 设置 socket 为非阻塞模式
static void setNonBlocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void Chat::Connect() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        return;
    }
    // 设置客户端 socket 为非阻塞模式
    setNonBlocking(clientSocket);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        // 非阻塞 connect 返回 -1 是正常情况，
        // 通过 select() 判断是否连通
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(clientSocket, &writefds);
        timeval tv;
        tv.tv_sec = 10; // 超时 10 秒
        tv.tv_usec = 0;
        int ret = select(clientSocket + 1, NULL, &writefds, NULL, &tv);
        if(ret <= 0) {
            std::cerr << "Error: Failed to connect to server." << std::endl;
            return;
        }
    }
    std::cout << "Connected to server." << std::endl;
}

void Chat::Send() {
    std::cin.getline(message, MAX_MESSAGE_LENGTH);
    char* cipherText = nullptr;
    int cipherTextLength = -1;

    if (strcmp(message, EXIT_COMMAND) == 0) {
        isRunning = false;
        exited = true;
    }

    des.Encrypt(message, strlen(message), cipherText, cipherTextLength);
    if (send(clientSocket, cipherText, cipherTextLength, 0) < 0) {
        std::cerr << "Error: Failed to send message." << std::endl;
        delete[] cipherText;
        return;
    }
    delete[] cipherText;
}

void Chat::ReceiveThread() {
    const char* info = isServer ? "Client" : "Server";
    char* plainText = nullptr;
    int plainTextLength = -1;
    
    while (isRunning) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        timeval tv;
        tv.tv_sec = 1;  // 超时 1 秒，避免长时间阻塞
        tv.tv_usec = 0;
        int ret = select(clientSocket + 1, &readfds, NULL, NULL, &tv);
        if(ret > 0 && FD_ISSET(clientSocket, &readfds)) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t len = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (len <= 0) {
                isRunning = false;
                if (!exited) {
                    std::cerr << "Error: Failed to receive message or connection closed." << std::endl;
                }
                break;
            }
            plainText = nullptr;
            plainTextLength = -1;
            des.Decrypt(buffer, len, plainText, plainTextLength);
        
            // 检查退出命令
            if (strcmp(plainText, EXIT_COMMAND) == 0) {
                isRunning = false;
                delete[] plainText;
                std::cout << info << " exited." << std::endl;
                break;
            }
        
            std::cout << info << ": " << plainText << std::endl;
            delete[] plainText;
        }
        else if(ret < 0) {
            isRunning = false;
            std::cerr << "Error: select() failed." << std::endl;
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
    
    // 创建 serverSocket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        return;
    }
    // 设置 serverSocket 为非阻塞
    setNonBlocking(serverSocket);
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 接受任意 IP

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Failed to bind." << std::endl;
        return;
    }
    
    if (listen(serverSocket, 1) < 0) {
        std::cerr << "Error: Failed to listen." << std::endl;
        return;
    }
    
    // 使用 select() 等待连接到来
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(serverSocket, &readfds);
    timeval tv;
    tv.tv_sec = 10;   // 等待10秒
    tv.tv_usec = 0;
    int ret = select(serverSocket + 1, &readfds, NULL, NULL, &tv);
    if(ret <= 0) {
        std::cerr << "Error: No incoming connection within timeout." << std::endl;
        return;
    }
    
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket < 0) {
        std::cerr << "Error: Failed to accept." << std::endl;
        return;
    } else {
        std::cout << "Client connected." << std::endl;
    }
    // 设置 clientSocket 为非阻塞模式
    setNonBlocking(clientSocket);

    // RSA 密钥生成（最多重试 3 次）
    const int N_RETRY = 3;
    for (int i = 0; i < N_RETRY; i++) {
        if (rsa.GenerateKey()) {
            std::cout << "RSA key generated successfully." << std::endl;
            break;
        }
        if (i == N_RETRY - 1) {
            std::cerr << "Error: Failed to generate RSA key." << std::endl;
            std::cerr << "Server Exiting..." << std::endl;
            return;
        } else {
            std::cerr << "Warning: Failed to generate RSA key. Retrying..." << std::endl;
        }
    }
    
    // 显示 RSA 详细配置信息
    rsa.PrintConfig();
    
    // 发送公钥和模数给客户端
    uint64_t e = rsa.GetPublicKey();
    uint64_t n = rsa.GetModulus();
    if (send(clientSocket, reinterpret_cast<const char*>(&e), sizeof(e), 0) < 0) {
        std::cerr << "Error: Failed to send public key." << std::endl;
        return;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&n), sizeof(n), 0) < 0) {
        std::cerr << "Error: Failed to send modulus." << std::endl;
        return;
    }
    
    // 使用 select() 等待客户端发送 DES 密钥（加密后的 DES key）
    FD_ZERO(&readfds);
    FD_SET(clientSocket, &readfds);
    tv.tv_sec = 5;  // 等待5秒
    tv.tv_usec = 0;
    ret = select(clientSocket + 1, &readfds, NULL, NULL, &tv);
    if(ret <= 0) {
        std::cerr << "Error: Timeout waiting for DES key." << std::endl;
        return;
    }
    
    uint64_t desKey_enc[8];
    if (recv(clientSocket, reinterpret_cast<char*>(desKey_enc), sizeof(desKey_enc), 0) < 0) {
        std::cerr << "Error: Failed to receive DES key." << std::endl;
        return;
    }
    
    uint8_t desKey[8];
    for (int i = 0; i < 8; i++) {
        desKey[i] = rsa.Decrypt(desKey_enc[i]);
    }
    des.SetKey((char*)desKey);
    
    std::cout << "Key exchange completed." << std::endl;
    std::cout << "You can start chatting now." << std::endl;
    
    auto chatLoop = [this]() {
        isRunning = true;
        receiveThread = std::thread(&Chat::ReceiveThread, this);
        while (isRunning) {
            Send();
        }
        Close();
    };
    
    chatLoop();
}

void Chat::RunClient() {
    isServer = false;
    Connect();
    
    des.RandomGenKey();
    uint8_t* desKey = des.GetKey();
    
    // 等待服务器发来公钥和模数
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(clientSocket, &readfds);
    timeval tv;
    tv.tv_sec = 5;   // 等待5秒
    tv.tv_usec = 0;
    int ret = select(clientSocket + 1, &readfds, NULL, NULL, &tv);
    if(ret <= 0) {
        std::cerr << "Error: Timeout waiting for public key and modulus." << std::endl;
        return;
    }
    
    uint64_t e, n;
    if (recv(clientSocket, reinterpret_cast<char*>(&e), sizeof(e), 0) < 0) {
        std::cerr << "Error: Failed to receive public key." << std::endl;
        return;
    }
    if (recv(clientSocket, reinterpret_cast<char*>(&n), sizeof(n), 0) < 0) {
        std::cerr << "Error: Failed to receive modulus." << std::endl;
        return;
    }
    
    uint64_t desKey_enc[8];
    for (int i = 0; i < 8; i++) {
        desKey_enc[i] = RSA::Encrypt((uint32_t)desKey[i], e, n);
    }
    delete[] desKey;
    
    if (send(clientSocket, reinterpret_cast<const char*>(desKey_enc), sizeof(desKey_enc), 0) < 0) {
        std::cerr << "Error: Failed to send DES key." << std::endl;
        return;
    }
    
    std::cout << "Key exchange completed." << std::endl;
    
    auto chatLoop = [this]() {
        isRunning = true;
        receiveThread = std::thread(&Chat::ReceiveThread, this);
        while (isRunning) {
            Send();
        }
        Close();
    };
    
    chatLoop();
}