#include "server.hpp"
#include "request.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

void Server::setup_socket() {
    // create the socket
    int socket_fd = socket(
        AF_INET, // IPv4
        SOCK_STREAM, // TCP
        0 // default
    );
    if (socket_fd == -1) {
        // failed to create socket
        throw std::runtime_error("failed to create socket");
    }
    this->socket_fd = socket_fd;

    // set socket options to allow reusing the address
    int reuseaddr_opt = 1;
    int sockopt_ret = setsockopt(
        this->socket_fd,
        SOL_SOCKET, // manage at the sockets API level
        SO_REUSEADDR, // reuse option name
        &reuseaddr_opt,
        sizeof(reuseaddr_opt)
    );
    if (sockopt_ret == -1) {
        // failed to set sockopt
        throw std::runtime_error("failed to set sockopt");
    }

    // bind the socket
    struct sockaddr_in addr = {
        AF_INET, // IPv4
        htons(this->port), // use network order
        {} // bind all network interfaces
    };
    int bind_ret = bind(
        this->socket_fd,
        (struct sockaddr *) &addr,
        sizeof(addr)
    );
    if (bind_ret == -1) {
        // failed to bind socket
        throw std::runtime_error("failed to bind socket");
    }

    // listen on the socket
    int listen_ret = listen(
        this->socket_fd,
        MAX_CONNECTIONS // number of connections that can be waiting at the same time
    );
    if (listen_ret == -1) {
        // failed to listen on socket
        throw std::runtime_error("failed to listen on socket");
    }
}

void Server::setup_io_uring() {
    // create the ring
    int queue_init_ret = io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
    if (queue_init_ret == -1) {
        throw std::runtime_error("failed to init io_uring queue");
    }
}

// delegate construction to Server(int port)
Server::Server() : Server(DEFAULT_PORT) {}

Server::Server(int port) {
    this->port = port;
    setup_socket();
    setup_io_uring();
}

Server::~Server() {
    close(this->socket_fd);
    io_uring_queue_exit(&this->ring);
}

void Server::start() {
    std::cout << "Server (fd=" << this->socket_fd << ")" << " is now ready at " << "http://0.0.0.0:" << this->port << std::endl;

    // create the first accept request
    add_accept_request();
    // submit sqe to the ring
    io_uring_submit(&this->ring);

    while (true) {
        int wait_cqe_ret = io_uring_wait_cqe(
            &this->ring,
            &this->cqe
        );
        if (wait_cqe_ret < 0) {
            throw std::runtime_error("failed to wait cqe");
        }

        // AT THIS POINT, WAIT FINISHED, CQE RECEIVED
        Connection *conn = (Connection *) io_uring_cqe_get_data(cqe);
        if (cqe->res < 0) {
            // ERROR occured
            add_close_request(conn);
            io_uring_submit(&this->ring);
            continue;
        }

        switch (conn->optype) {
            case OpType::ACCEPT_CONNECTION:
                add_accept_request();

                // set connection fd to client fd
                conn->fd = cqe->res;
                add_recv_request(conn);

                io_uring_submit(&this->ring);
                break;
            case OpType::RECV_REQUEST:
                // nothing was sent or client closed
                if (cqe->res == 0) {
                    add_close_request(conn);
                    break;
                }

                // receive until all required bytes are received
                if (is_valid_request(conn)) {
                    handle_request(conn, cqe->res);
                    prepare_response(conn);
                    add_send_request(conn);
                } else {
                    add_recv_request(conn);
                }

                io_uring_submit(&this->ring);
                break;
            case OpType::SEND_RESPONSE:
                std::cout << "fd=" << conn->fd << " bytes_sent=" << cqe->res << " SEND: " << conn->response->status_line << std::endl;

                if (conn->response->keep_alive) {
                    conn->request.reset();
                    conn->response.reset();
                    // go back to receiving
                    add_recv_request(conn);
                } else {
                    add_close_request(conn);
                }

                io_uring_submit(&this->ring);
                break;
            case OpType::CLOSE_CONNECTION:
                delete conn;
                break;
        }

        io_uring_cqe_seen(&this->ring, this->cqe);
    }
}

void Server::add_accept_request() {
    this->sqe = io_uring_get_sqe(&ring);
    io_uring_prep_accept(
        sqe,
        this->socket_fd,
        nullptr,
        nullptr,
        0
    );
    io_uring_sqe_set_data(
        this->sqe,
        new Connection{ this->socket_fd, OpType::ACCEPT_CONNECTION }
    );
}

void Server::add_recv_request(Connection *conn) {
    // issue the recv
    conn->optype = OpType::RECV_REQUEST;
    struct io_uring_sqe *recv_sqe = io_uring_get_sqe(&this->ring);
    io_uring_prep_recv(
        recv_sqe,
        conn->fd,
        conn->request_buffer.data(),
        RECV_BUFFER_SIZE,
        0
    );
    io_uring_sqe_set_data(recv_sqe, conn);
}

void Server::add_send_request(Connection *conn) {
    // issue the send
    conn->optype = OpType::SEND_RESPONSE;
    struct io_uring_sqe *send_sqe = io_uring_get_sqe(&this->ring);
    io_uring_prep_send(
        send_sqe,
        conn->fd,
        conn->response->response_str.data(),
        conn->response->response_str.size(),
        0
    );
    io_uring_sqe_set_data(send_sqe, conn);
}

void Server::add_close_request(Connection *conn) {
    // issue the close
    conn->optype = OpType::CLOSE_CONNECTION;
    struct io_uring_sqe *close_sqe = io_uring_get_sqe(&this->ring);
    io_uring_prep_close(close_sqe, conn->fd);
    io_uring_sqe_set_data(close_sqe, conn);
}

bool Server::is_valid_request(Connection *conn) {
    std::string raw_request(conn->request_buffer.data());
    return raw_request.find(CRLF CRLF, 0) != std::string::npos;
}

void Server::handle_request(Connection *conn, int bytes_recv) {
    // parse the request
    std::string raw_request(conn->request_buffer.data(), bytes_recv);
    conn->request = std::make_unique<Request>();
    conn->request->parse_request(raw_request);
    std::cout << "fd=" << conn->fd << " bytes_recv=" << bytes_recv << " RECV: " << conn->request->method << " " << conn->request->uri << std::endl;
}

void Server::prepare_response(Connection *conn) {
    // build the response
    conn->response = std::make_unique<Response>();
    conn->response->build(*conn->request);
    conn->response->prepare();
}
