#include "server.hpp"
#include "request.hpp"
#include "response.hpp"
#include <array>
#include <cstring>
#include <iostream>
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

Server::Server() {
    this->port = DEFAULT_PORT;
    setup_socket();
}

Server::Server(int port) {
    this->port = port;
    setup_socket();
}

Server::~Server() {
    close(this->socket_fd);
}

void Server::start() {
    std::cout << "Server is now ready at " << "http://0.0.0.0:" << this->port << std::endl;
    while (true) {
        // accept connection from client
        int client_fd = accept(
            this->socket_fd,
            0, // for additional client information
            0 // for additional client information
        );
        if (client_fd == -1) {
            // failed to accept connection
            throw std::runtime_error("failed to accept connection");
        }

        // receive request from client
        std::array<char, RECV_BUFFER_SIZE> request_buffer{};
        int bytes_recv = recv(
            client_fd,
            request_buffer.data(), // gives a pointer to the actual characters
            RECV_BUFFER_SIZE,
            0 // flags
        );
        if (bytes_recv == -1) {
            // failed to receive bytes
            throw std::runtime_error("failed to receive bytes");
        }

        // parse the request
        Request request;
        std::string raw_request(request_buffer.data(), bytes_recv);
        
        request.parse_request(raw_request);

        // print request
        // std::cout << request << std::endl;
        std::cout << "fd=" << client_fd << " RECV: " << request.method << " " << request.uri << std::endl;

        // send string to client
        Response response;
        response.build(request);
        response.prepare();
        // std::cout << "===RESPONSE===" << std::endl;
        // std::cout << response.response_str << std::endl;
        int bytes_sent = send(
            client_fd,
            response.response_str.data(), // gives a pointer to the actual characters
            response.response_str.size(),
            0
        );
        if (bytes_sent == -1) {
            // failed to send
            throw std::runtime_error("failed to send bytes");
        }
        std::cout << "fd=" << client_fd << " SEND: " << response.status_line << std::endl;

        // close the socket
        close(client_fd);
    }
}
