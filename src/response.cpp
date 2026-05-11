#include "response.hpp"
#include "filecache.hpp"
#include "http_status_codes.hpp"
#include "request.hpp"
#include <string>


void Response::build(Request& req, FileCache& fc) {
    bool in_cache = fc.contains(req.uri);
    CachedFile file;
    if (in_cache) {
        this->status_code = HTTPStatusCode::OK;
        file = fc.get(req.uri);
    } else if (!in_cache && req.uri == "/") {
        this->status_code = HTTPStatusCode::OK;
        file = fc.get("/index.html");
    } else {
        this->status_code = HTTPStatusCode::NOT_FOUND;
        file = fc.get("/404.html");
    }

    // build status-line
    this->response_str.append("HTTP/1.1 ");
    this->response_str.append(std::to_string(static_cast<int>(this->status_code)));
    this->response_str.append(" ");
    this->response_str.append(status_code_to_string[this->status_code]);
    this->response_str.append(CRLF);

    // build headers
    this->response_str.append("Content-Type: ");
    this->response_str.append(file.content_type);
    this->response_str.append(CRLF);
    
    this->response_str.append("Content-Length: ");
    this->response_str.append(std::to_string(file.content_length));
    this->response_str.append(CRLF);

    this->response_str.append("Connection: ");
    this->response_str.append(req.keep_alive ? "keep-alive" : "close");
    this->response_str.append(CRLF CRLF);

    // build message-body
    this->response_str.append(file.body);
}
