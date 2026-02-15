#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
// #include <unistd.h>

const int MAX_CONNECTIONS = 5;

#define EXT_ERR_INIT_SOCKET 1
#define EXT_ERR_INIT_LISTEN 2
#define EXT_ERR_INIT_ACCEPT 3

int main(void) {
    // create socket
    // AF_INET: IPv4; SOCK_STREAM: TCP socket
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == -1) {
        return EXT_ERR_INIT_SOCKET;
    }
    // bind the socket to a port
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); // convert 8080 to network byte order
    serverAddr.sin_addr.s_addr = INADDR_ANY; // accept connection on any IP
    bind(server, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    // listen on the socket
    if (listen(server, MAX_CONNECTIONS) == -1) {
        return EXT_ERR_INIT_LISTEN;
    }
    while (true) {
        // accept connection
        int client = accept(server, nullptr, nullptr);
        if (client == -1) {
            return EXT_ERR_INIT_ACCEPT;
        }
        char buffer[1024] = {0};
        recv(client, buffer, sizeof(buffer), 0);
        std::cout << "Message from client: " << buffer << std::endl;
    }
    // close(server);
}
