#include <cctype>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <array>

#define DEFAULT_PORT 8080
#define RECV_BUFFER_SIZE 1024
#define MAX_CONNECTIONS 10
#define CRLF "\r\n"
#define HEADER_DELIM ": "

void parse_request_line(std::string);
void parse_headers(std::string);
void parse_body(std::string);
std::vector<std::string> split(std::string, const char *);

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
    std::string raw_request(request_buffer.data(), bytes_recv);
    
    size_t start_pos = 0;

    // parse request_line
    size_t rl_crlf_idx = raw_request.find(CRLF, start_pos);
    if (rl_crlf_idx == std::string::npos) {
        // incomplete or malformed request
        perror("unable to parse request-line due to incomplete or malformed request");
        return 1;
    }
    std::string raw_request_line = raw_request.substr(start_pos, rl_crlf_idx);
    parse_request_line(raw_request_line);

    // parse headers
    // offset starting position to not include request_line
    start_pos = rl_crlf_idx + strlen(CRLF);
    // get the end of the headers
    // adjacent string literals are concatenated by the compiler at compile time
    size_t hdrs_end_idx = raw_request.find(CRLF CRLF, start_pos);
    if (hdrs_end_idx == std::string::npos) {
        // incomplete or malformed request
        perror("unable to parse headers due to incomplete or malformed request");
        return 1;
    }
    std::string raw_request_headers = raw_request.substr(start_pos, hdrs_end_idx - start_pos);
    parse_headers(raw_request_headers);

    // parse body
    start_pos = hdrs_end_idx + strlen(CRLF CRLF);
    std::string raw_request_body = raw_request.substr(start_pos);
    parse_body(raw_request_body);

    // send string to client
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
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


void parse_request_line(std::string raw_request_line) {
    std::vector<std::string> components = split(raw_request_line, " ");
    // method       = components[0];
    // request_uri  = components[1];
    // http_version = components[2];
}


void parse_headers(std::string raw_request_headers) {
    std::vector<std::string> headers = split(raw_request_headers, CRLF);

    std::unordered_map<std::string, std::string> header_map;

    for (std::string raw_header : headers) {
        size_t delim_idx = raw_header.find(HEADER_DELIM, 0);
        header_map.emplace(
            raw_header.substr(0, delim_idx),
            raw_header.substr(delim_idx + strlen(HEADER_DELIM))
        );
    }
}


void parse_body(std::string raw_request_body) {
    // temp parse body
}


std::vector<std::string> split(std::string str, const char *delim) {
    size_t start_pos = 0;
    std::vector<std::string> result;
    while (true) {
        size_t curr_pos = str.find(delim, start_pos);
        if (curr_pos == std::string::npos) {
            result.push_back(str.substr(start_pos));    
            break;
        }
        result.push_back(str.substr(start_pos, curr_pos - start_pos));
        start_pos = curr_pos + strlen(delim);
    }
    return result;
}
