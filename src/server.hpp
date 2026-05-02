#pragma once
#include "liburing.h"
#include "request.hpp"
#include "response.hpp"
#include <array>
#include <memory>

#define RECV_BUFFER_SIZE 1024
#define DEFAULT_PORT 8080
#define MAX_CONNECTIONS 10
#define QUEUE_DEPTH 8

enum class OpType {
    ACCEPT_CONNECTION,
    RECV_REQUEST,
    SEND_RESPONSE,
    CLOSE_CONNECTION,
};

class Connection {
public:
    std::array<char, RECV_BUFFER_SIZE> request_buffer{};
    Connection(int server_fd, OpType optype);
    int fd;
    OpType optype;
    std::unique_ptr<Request> request;
    std::unique_ptr<Response> response;
};

inline Connection::Connection(int server_fd, OpType optype) {
    this->fd = server_fd;
    this->optype = optype;
}

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
    struct io_uring ring;
    struct io_uring_sqe *sqe; // submission queue event
    struct io_uring_cqe *cqe; // consumption queue event
    void setup_socket();
    void setup_io_uring();
    void add_accept_request();
    void add_recv_request(Connection *conn, int client_fd);
    void add_send_request(Connection *conn, int bytes_recv);
    void add_close_request(Connection *conn);
    void add_close_request(Connection *conn, int bytes_sent);
};
