#include <iostream>
#include "chat.h"

int main() {
    Chat chat;
    char isServer;
    std::cout << "Are you Server or Client? (s/c): ";
    std::cin >> isServer;
    if (isServer == 's') {
        chat.RunServer();
    } else if (isServer == 'c') {
        chat.RunClient();
    } else {
        std::cerr << "Error: Invalid input." << std::endl;
    }
    return 0;
}