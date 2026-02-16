#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    const char* message = "i'm the client";
    send(clientSocket, message, strlen(message), 0);

    accept(clientSocket, nullptr, nullptr);

    char buffer[1024];
    recv(clientSocket, buffer, strlen(buffer), 0);
    std::cout << "Server response: " << buffer << std::endl;
    close(clientSocket);
}
