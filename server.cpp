#include <asm-generic/socket.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <string_view>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <array>

#define DEFAULT_PORT 8080
#define RECV_BUFFER_SIZE 1024
#define MAX_CONNECTIONS 10
#define CRLF "\r\n"

void parse_request_line(std::string_view buffer);
std::vector<std::string_view> split(std::string_view str, const char *delim);

int main(int argc, char *argv[]) {
    // create the socket
    int socket_fd = socket(
        AF_INET, // IPv4
        SOCK_STREAM, // TCP
        0 // default
    );
    if (socket_fd == -1) {
        // failed to create socket
        perror("failed to create socket");
        return 1;
    }

    // set socket options to allow reusing the address
    int reuseaddr_opt = 1;
    int sockopt_ret = setsockopt(
        socket_fd,
        SOL_SOCKET, // manage at the sockets API level
        SO_REUSEADDR, // reuse option name
        &reuseaddr_opt,
        sizeof(reuseaddr_opt)
    );
    if (sockopt_ret == -1) {
        // failed to set sockopt
        perror("failed to set sockopt");
        return 1;
    }

    // bind the socket
    struct sockaddr_in addr = {
        AF_INET, // IPv4
        htons(DEFAULT_PORT), // use network order
        {} // bind all network interfaces

    };
    int bind_ret = bind(
        socket_fd,
        (struct sockaddr *) &addr,
        sizeof(addr)
    );
    if (bind_ret == -1) {
        // failed to bind socket
        perror("failed to bind socket");
        return 1;
    }

    // listen on the socket
    int listen_ret = listen(
        socket_fd,
        MAX_CONNECTIONS // number of connections that can be waiting at the same time
    );
    if (listen_ret == -1) {
        // failed to listen on socket
        perror("failed to listen on socket");
        return 1;
    }

    // accept connection from client
    int client_fd = accept(
        socket_fd,
        0, // for additional client information
        0 // for additional client information
    );
    if (client_fd == -1) {
        // failed to accept connection
        perror("failed to accept connection");
        return 1;
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
        perror("failed to receive bytes");
        return 1;
    }

    // parse the request
    /**
    request-line<CRLF>
    headers<CRLF>
    <CRLF>
    message-body
     */
    parse_request_line(std::string_view(request_buffer.data(), bytes_recv));

    // send string to client
    std::string_view response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
    int bytes_sent = send(
        client_fd,
        response.data(), // gives a pointer to the actual characters
        response.size(),
        0
    );
    if (bytes_sent == -1) {
        // failed to send
        perror("failed to send bytes");
        return 1;
    }

    // close the sockets
    close(client_fd);
    close(socket_fd);
    return 0;
}
void parse_request_line(std::string_view buffer) {
    size_t crlf_idx = buffer.find(CRLF, 0);
    std::string_view request_line = buffer.substr(0, crlf_idx);
    std::vector<std::string_view> components = split(request_line, " ");
    // method       = components[0];
    // request_uri  = components[1];
    // http_version = components[2];
}

std::vector<std::string_view> split(std::string_view str, const char *delim) {
    size_t start_pos = 0;
    std::vector<std::string_view> result;
    while (true) {
        size_t curr_pos = str.find(delim, start_pos);
        if (curr_pos == std::string_view::npos) {
            result.push_back(str.substr(start_pos, curr_pos - start_pos));    
            break;
        }
        result.push_back(str.substr(start_pos, curr_pos - start_pos));
        start_pos = curr_pos + strlen(delim);
    }
    return result;
}
