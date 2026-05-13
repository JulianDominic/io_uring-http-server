#pragma once
#include <string>
#include "filecache.hpp"
#include "http_status_codes.hpp"
#include "request.hpp"
#include <sys/stat.h>

#define URI_DIR "public"
#define RES_SIZE 8192

class Response {
public:
    Response() { response_str.reserve(RES_SIZE); }
    HTTPStatusCode status_code;
    std::string response_str;
    void build(Request& req, FileCache& fc);
    void reset();
};

inline void Response::reset() {
    this->response_str.clear();
}
