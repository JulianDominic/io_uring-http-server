#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
// #include <unistd.h>

#include <liburing.h>

#define MAX_CONNECTIONS 4096
#define ENTRIES 1024 // must be a power of 2
#define MAX_BUFFER_SIZE 2048

#define EXT_ERR_PORT_MISSING -1

#define EXT_ERR_INIT_SOCKET 1
#define EXT_ERR_INIT_LISTEN 2
#define EXT_ERR_INIT_ACCEPT 3
#define EXT_ERR_INIT_BIND 4

#define EXT_ERR_INIT_IO_URING_QUEUE 5
#define EXT_ERR_WAIT_CQE 6

enum class OpType {
    ACCEPT,
    READ_REQUEST,
    WRITE_RESPONSE
};

struct Connection {
    int fd;
    OpType type;
    char buffer[MAX_BUFFER_SIZE];
    int file_fd;
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        perror("Port number required: ./server <port>\n");
        return EXT_ERR_PORT_MISSING;
    }
    // create socket
    // AF_INET: IPv4; SOCK_STREAM: TCP socket
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == -1) {
        perror("Failed to start the socket\n");
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
        perror("Failed to bind the socket to the port\n");
        return EXT_ERR_INIT_BIND;
    }
    // listen on the socket
    if (listen(server, MAX_CONNECTIONS) == -1) {
        perror("Failed to listen on the socket\n");
        return EXT_ERR_INIT_LISTEN;
    }
    std::cout << "server is now listening on port: " << port << std::endl;

    // create the ring
    struct io_uring ring;
    if (io_uring_queue_init(ENTRIES, &ring, 0) < 0) {
        perror("Failed to create the io_uring ring\n");
        return EXT_ERR_INIT_IO_URING_QUEUE;
    }

    // create the buffers
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;

    // submit the first accept
    sqe = io_uring_get_sqe(&ring);
    Connection *conn = new Connection{ server, OpType::ACCEPT };
    // multishot makes it such that after accepting a connection, I don't need to resubmit a new accept SQE
    io_uring_prep_multishot_accept(sqe, server, nullptr, nullptr, 0);
    io_uring_sqe_set_data(sqe, conn);
    io_uring_submit(&ring);

    while (true) {
        if (io_uring_wait_cqe(&ring, &cqe) < 0) {
            perror("Failed to wait for CQE\n");
            return EXT_ERR_WAIT_CQE;
        }
        Connection *conn = (Connection *)cqe->user_data;

        switch (conn->type) {
            case OpType::ACCEPT: {
                // create a new connection struct instead of modifying the existing one because it is a multishot accept
                Connection *clientConn = new Connection{ cqe->res, OpType::READ_REQUEST };
                struct io_uring_sqe *readSQE = io_uring_get_sqe(&ring);
                io_uring_prep_read(readSQE, clientConn->fd, clientConn->buffer, MAX_BUFFER_SIZE, 0);
                io_uring_sqe_set_data(readSQE, clientConn);
                if (io_uring_submit(&ring) < 0) {
                    perror("Failed to submit READ_REQUEST to SQE\n");
                }
                break;
            }
            case OpType::READ_REQUEST: {
                std::cout << conn->buffer << std::endl;
                conn->type = OpType::WRITE_RESPONSE;
                struct io_uring_sqe *writeSQE = io_uring_get_sqe(&ring);
                io_uring_prep_write(writeSQE, conn->fd, conn->buffer, cqe->res, 0);
                io_uring_sqe_set_data(writeSQE, conn);
                if (io_uring_submit(&ring) < 0) {
                    perror("Failed to submit WRITE_RESPONSE to SQE\n");
                }
                break;
            }
            case OpType::WRITE_RESPONSE: {
                // std::cout << "WRITE_RESPONSE" << std::endl;
                break;
            }
        }
        io_uring_cqe_seen(&ring, cqe);
    }
}
