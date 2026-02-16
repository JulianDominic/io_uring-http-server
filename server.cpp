#include <string.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

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
    WRITE_RESPONSE,
    CLOSE,
};

struct Connection {
    int fd;
    OpType type;
    char buffer[MAX_BUFFER_SIZE];
    int file_fd;
};

void read_request(struct io_uring &ring, signed int &fd);
void write_response(struct io_uring &ring, Connection *&conn, int &bytesRead);
void close_connection(struct io_uring &ring, Connection *&conn);

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
        if (cqe->res <= 0 && conn->type != OpType::CLOSE && conn->type != OpType::ACCEPT) {
            close_connection(ring, conn);
            io_uring_cqe_seen(&ring, cqe);
            continue;
        }
        switch (conn->type) {
            case OpType::ACCEPT: {
                std::cout << "New FD: " << cqe->res << std::endl;
                read_request(ring, cqe->res);
                break;
            }
            case OpType::READ_REQUEST: {
                std::cout << conn->buffer << std::endl;
                write_response(ring, conn, cqe->res);
                break;
            }
            case OpType::WRITE_RESPONSE: {
                close_connection(ring, conn);
                break;
            }
            case OpType::CLOSE: {
                std::cout << "Closed FD: " << conn->fd << std::endl;
                delete conn;
                break;
            }
        }
        io_uring_cqe_seen(&ring, cqe);
    }
}

void read_request(struct io_uring &ring, signed int &fd) {
    // create a new connection struct instead of modifying the existing one because it is a multishot accept
    Connection *clientConn = new Connection{ fd, OpType::READ_REQUEST };
    memset(clientConn->buffer, 0, MAX_BUFFER_SIZE);
    struct io_uring_sqe *readSQE = io_uring_get_sqe(&ring);
    io_uring_prep_read(readSQE, clientConn->fd, clientConn->buffer, MAX_BUFFER_SIZE, 0);
    io_uring_sqe_set_data(readSQE, clientConn);
    if (io_uring_submit(&ring) < 0) {
        perror("Failed to submit READ_REQUEST as SQE\n");
    }
}

void write_response(struct io_uring &ring, Connection *&conn, int &bytesRead) {
    conn->type = OpType::WRITE_RESPONSE;
    struct io_uring_sqe *writeSQE = io_uring_get_sqe(&ring);
    io_uring_prep_write(writeSQE, conn->fd, conn->buffer, bytesRead, 0);
    io_uring_sqe_set_data(writeSQE, conn);
    if (io_uring_submit(&ring) < 0) {
        perror("Failed to submit WRITE_RESPONSE as SQE\n");
    }
}

void close_connection(struct io_uring &ring, Connection *&conn) {
    conn->type = OpType::CLOSE;
    struct io_uring_sqe *closeSQE = io_uring_get_sqe(&ring);
    io_uring_prep_close(closeSQE, conn->fd);
    io_uring_sqe_set_data(closeSQE, conn);
    if (io_uring_submit(&ring) < 0) {
        perror("Failed to submit CLOSE_FD as SQE\n");
    }
}
