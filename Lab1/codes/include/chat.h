#ifndef DES_CHAT_CHAT_H
#define DES_CHAT_CHAT_H

#ifdef _WIN32
#include <winsock2.h>
#define close(s) closesocket(s)
typedef SSIZE_T ssize_t;
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <thread>
#include <atomic>
#include "DES_Operation.h"

#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 8888
#define MAX_MESSAGE_LENGTH 512
#define EXIT_COMMAND "quit"
#define KEY "Luhaozhe"

class Chat {
private:
    bool isServer = false;
    int serverSocket = -1;
    int clientSocket = -1;
    const char* serverIp = DEFAULT_SERVER_IP;
    int serverPort = DEFAULT_SERVER_PORT;
    char message[MAX_MESSAGE_LENGTH] = {0};
    char buffer[MAX_MESSAGE_LENGTH] = {0};
    std::atomic<bool> isRunning = false;
    std::thread receiveThread;
    DesOp des;

    void Init();
    void Connect();
    void Send();
    void ReceiveThread();
    void Close();
    bool CreateSocket(int& sock);
    bool BindSocket(int sock, sockaddr_in& serverAddr);

public:
    Chat();
    ~Chat();
    void RunServer();
    void RunClient();
};

#endif