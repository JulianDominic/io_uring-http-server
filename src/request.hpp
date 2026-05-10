#pragma once
#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>
#include "http_method.hpp"

#define CRLF "\r\n"
#define HEADER_DELIM ": "

class Request {
public:
    bool keep_alive;
    HTTPMethod method;
    std::string_view uri;
    std::unordered_map<std::string, std::string> headers;
    std::string_view body;
    Request(){};
    void parse_request(std::string_view);
    void parse_request_line(std::string_view);
    void parse_headers(std::string_view);
    void parse_body(std::string_view);
};

// this function lets me print Request structs with just std::cout << request << std::endl;
inline std::ostream& operator<<(std::ostream& os, const Request& request) {
    os << "METHOD: " << request.method << "\n";
    os << "URI: " << request.uri << "\n";
    os << "===HEADERS===" << "\n";
    for (auto [key, value] : request.headers) {
        os << key << ": " << value << "\n";
    }
    os << "=============" << "\n";
    os << "BODY:\n" << request.body;
    return os;
}
