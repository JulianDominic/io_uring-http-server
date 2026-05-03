#pragma once
#include <string>
#include <unordered_map>
#include "http_status_codes.hpp"
#include "request.hpp"
#include <stdexcept>
#include <sys/stat.h>

#define URI_DIR "public"

class Response {
    HTTPStatusCode status_code;
    std::string path;
    int content_length;
    void build_status_line(std::string uri);
    void build_headers(std::unordered_map<std::string, std::string> request_headers);
    void build_message_body();
public:
    std::string response_str;
    std::string status_line;
    std::unordered_map<std::string, std::string> headers;
    std::string message_body;
    bool keep_alive;
    Response(){}
    void build(Request& req);
    void prepare();
};

inline int get_file_size(std::string path) {
    struct stat stat_buffer;
    int stat_ret = stat(
        path.data(),
        &stat_buffer
    );
    if (stat_ret == -1) {
        std::cout << errno << std::endl;
        throw std::runtime_error("failed to get file stat");
    }
    return stat_buffer.st_size;
}
