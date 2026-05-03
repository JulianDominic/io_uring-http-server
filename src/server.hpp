#pragma once
#include "liburing.h"
#include "request.hpp"
#include "response.hpp"
#include <array>
#include <cstddef>
#include <memory>

#define RECV_BUFFER_SIZE 1024
#define DEFAULT_PORT 8080
#define MAX_CONNECTIONS 10000
#define QUEUE_DEPTH 1024

enum class OpType {
    ACCEPT_CONNECTION,
    RECV_REQUEST,
    SEND_RESPONSE,
    CLOSE_CONNECTION,
};

class Connection {
public:
    std::array<char, RECV_BUFFER_SIZE> request_buffer{};
    size_t recv_len = 0;
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
    // struct io_uring_sqe *sqe; // submission queue event
    struct io_uring_cqe *cqe; // consumption queue event
    void setup_socket();
    void setup_io_uring();
    void add_accept_request();
    void add_recv_request(Connection *conn);
    void add_send_request(Connection *conn);
    void add_close_request(Connection *conn);
    size_t find_headers_end(Connection *conn);
    void handle_request(Connection *conn);
    void compact_buffer(Connection* conn, size_t consumed);
    void prepare_response(Connection *conn);
};
