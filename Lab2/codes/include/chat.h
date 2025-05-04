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
#include "RSA_Operation.h"//added

#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 8888
#define MAX_MESSAGE_LENGTH 512
#define EXIT_COMMAND "quit"
#define KEY "Luhaozhe"

class Chat {
    private:
        bool isServer;
        int serverSocket;
        int clientSocket;
        const char* serverIp;
        int serverPort;
        char message[MAX_MESSAGE_LENGTH];
        char buffer[MAX_MESSAGE_LENGTH];
        std::atomic<bool> isRunning;
        std::atomic<bool> exited;
        std::thread receiveThread;
        DesOp des;
        RSA rsa; //added
        void Init();
        void Connect();
        void Send();
        void ReceiveThread();
        void Close();
    
    public:
        Chat();
        ~Chat();
        void RunServer();
        void RunClient();
    };

#endif