#include "response.hpp"
#include "http_status_codes.hpp"
#include "request.hpp"
#include <filesystem>
#include <fstream>
#include <utility>

void Response::build_status_line(std::string uri) {
    this->path = URI_DIR + uri;
    // check if the uri exists 
    if (uri == "/") {
        this->path = URI_DIR "/index.html";
        this->status_code = HTTPStatusCode::OK;
    } else if (std::filesystem::exists(this->path)) {
        this->status_code = HTTPStatusCode::OK;
    } else {
        this->status_code = HTTPStatusCode::NOT_FOUND;
        this->path = URI_DIR "/404.html";
    }
    // make the status-line
    // oss lets me combine strings with other data types
    std::ostringstream oss;
    oss << "HTTP/1.1" << " " << std::to_underlying(this->status_code) << " " << status_code_to_string[this->status_code];
    this->status_line = oss.str();
}

void Response::build_headers() {
    // TODO: handle other types
    std::string content_type = "text/html";

    // content-type header
    this->headers.emplace("Content-Type", content_type);

    // content-length header
    this->content_length = get_file_size(this->path);
    this->headers.emplace("Content-Length", std::to_string(this->content_length));

    // connection header
    // TODO: persistent HTTP
    this->headers.emplace("Connection", "close");
}

void Response::build_message_body() {
    // read file into string
    std::ifstream file(this->path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    this->message_body = buffer.str();
}

void Response::build(Request& req) {
    // build status-line
    Response::build_status_line(req.uri);
    // build headers
    Response::build_headers();
    // build message-body
    Response::build_message_body();
}

void Response::prepare() {
    std::ostringstream oss;
    // status-line
    oss << this->status_line << CRLF;
    // headers
    for (auto [key, value] : this->headers) {
        oss << key << ": " << value << CRLF;
    }
    oss << CRLF;
    // body
    oss << this->message_body;
    this->response_str = oss.str();
}
