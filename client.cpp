#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

char buffer[1024];

int main(void) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("failed to connect to the server\n");
        return 1;
    }
    const char* message = "i'm the client";
    send(clientSocket, message, strlen(message), 0);
    while (true) {
        int bytes_recv = recv(clientSocket, buffer, 2048, 0);
        if (bytes_recv > 0) {
            std::cout << "Server response: " << buffer << std::endl;
            break;        
        }
    }
    close(clientSocket);
}
