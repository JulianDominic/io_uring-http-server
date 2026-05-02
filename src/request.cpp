#include "request.hpp"
#include <cstring>
#include <stdexcept>
#include "utils.hpp"

void Request::parse_request(std::string raw_request) {
    /**
    request-line<CRLF>
    headers<CRLF>
    <CRLF>
    message-body
     */
    size_t start_pos = 0;

    // parse request_line
    size_t rl_crlf_idx = raw_request.find(CRLF, start_pos);
    if (rl_crlf_idx == std::string::npos) {
        // incomplete or malformed request
        throw std::runtime_error("unable to parse request-line due to incomplete or malformed request-line");
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
        throw std::runtime_error("unable to parse headers due to incomplete or malformed request headers");
    }
    std::string raw_request_headers = raw_request.substr(start_pos, hdrs_end_idx - start_pos);
    parse_headers(raw_request_headers);

    // parse body
    start_pos = hdrs_end_idx + strlen(CRLF CRLF);
    std::string raw_request_body = raw_request.substr(start_pos);
    parse_body(raw_request_body);
}

void Request::parse_request_line(std::string raw_request_line) {
    std::vector<std::string> components = split(raw_request_line, " ");
    if (components.size() != 3) {
        throw std::runtime_error("malformed request-line");
    }
    
    // check if method is supported
    if (!method_str_to_enum.contains(components[0])) {
        throw std::runtime_error("http method is not supported or invalid");
    }

    // check version
    if (components[2] != "HTTP/1.1") {
        throw std::runtime_error("wrong http version received");
    }

    this->method  = method_str_to_enum[components[0]];
    this->uri     = components[1];
    this->version = components[2];
}

void Request::parse_headers(std::string raw_request_headers) {
    std::vector<std::string> headers = split(raw_request_headers, CRLF);

    std::unordered_map<std::string, std::string> header_map;

    for (std::string raw_header : headers) {
        size_t delim_idx = raw_header.find(HEADER_DELIM, 0);
        header_map.emplace(
            raw_header.substr(0, delim_idx),
            raw_header.substr(delim_idx + strlen(HEADER_DELIM))
        );
    }
    this->headers = header_map;
}

void Request::parse_body(std::string raw_request_body) {
    this->body = raw_request_body;
}
