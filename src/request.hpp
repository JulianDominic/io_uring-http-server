#pragma once
#include "http_method.hpp"

#define CRLF "\r\n"
#define HEADER_DELIM ": "

class Request {
public:
    bool keep_alive = true;
    HTTPMethod method;
    std::string_view uri;
    Request(){};
    void parse_request(std::string_view);
    void parse_request_line(std::string_view);
    void parse_headers(std::string_view);
    void reset();
};


inline void Request::reset() {
    this->method = HTTPMethod::UNKNOWN;
    keep_alive = true;
    uri = {};
}
