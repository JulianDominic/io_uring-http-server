#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
// #include <unistd.h>

#define MAX_CONNECTIONS 4096
#define EXT_ERR_INIT_SOCKET 1
#define EXT_ERR_INIT_LISTEN 2
#define EXT_ERR_INIT_ACCEPT 3
#define EXT_ERR_INIT_BIND 4
#define EXT_ERR_PORT_MISSING 5

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Port number required: ./server <port>" << std::endl;
        return EXT_ERR_PORT_MISSING;
    }
    // create socket
    // AF_INET: IPv4; SOCK_STREAM: TCP socket
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == -1) {
        return EXT_ERR_INIT_SOCKET;
    }
    const int value = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

    // bind the socket to a port
    int port = strtol(argv[1], NULL, 10);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port); // convert 8080 to network byte order
    serverAddr.sin_addr.s_addr = INADDR_ANY; // accept connection on any IP
    if (bind(server, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        return EXT_ERR_INIT_BIND;
    }
    // listen on the socket
    if (listen(server, MAX_CONNECTIONS) == -1) {
        return EXT_ERR_INIT_LISTEN;
    }
    std::cout << "server is now listening on port: " << port << std::endl;

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
