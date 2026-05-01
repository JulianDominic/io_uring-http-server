#pragma once
#define RECV_BUFFER_SIZE 1024
#define DEFAULT_PORT 8080
#define MAX_CONNECTIONS 10

class Server {
public:
    int port;
    Server();
    Server(int port);
    ~Server(); // destructor
    Server(const Server&) = delete;             // disable copy constructor
    Server& operator=(const Server&) = delete;  // disable copy assignment
    void start();
private:
    int socket_fd;
    void setup_socket();
};
