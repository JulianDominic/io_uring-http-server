#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include "http_method.hpp"

#define CRLF "\r\n"
#define HEADER_DELIM ": "

class Request {
public:
    HTTPMethod method;
    std::string uri;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    Request(){};
    void parse_request(std::string);
    void parse_request_line(std::string);
    void parse_headers(std::string);
    void parse_body(std::string);
};

// this function lets me print Request structs with just std::cout << request << std::endl;
inline std::ostream& operator<<(std::ostream& os, const Request& request) {
    os << "METHOD: " << request.method << "\n";
    os << "URI: " << request.uri << "\n";
    os << "VERSION: " << request.version << "\n";
    os << "===HEADERS===" << "\n";
    for (auto [key, value] : request.headers) {
        os << key << ": " << value << "\n";
    }
    os << "=============" << "\n";
    os << "BODY:\n" << request.body;
    return os;
}
