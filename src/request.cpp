#include "request.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include "http_method.hpp"

void Request::parse_request(std::string_view raw_request) {
    /**
    request-line<CRLF>
    headers<CRLF>
    <CRLF>
    message-body
     */
    
    // find the first CRLF for the request-line
    size_t start_pos = 0;
    size_t end_pos = raw_request.find(CRLF);
    if (end_pos == std::string::npos) {
        throw std::runtime_error("malformed request-line");
    }
    std::string_view request_line = raw_request.substr(start_pos, end_pos);
    parse_request_line(request_line);
    
    // find the end of the headers
    start_pos = end_pos + 2; // CRLF = 2 bytes
    end_pos = raw_request.find(CRLF CRLF);
    if (end_pos == std::string::npos) {
        throw std::runtime_error("malformed headers");
    }
    // +2 to include the last header's CRLF
    std::string_view headers = raw_request.substr(start_pos, end_pos - start_pos + 2);
    parse_headers(headers);

    // parse the body
    start_pos = end_pos + 4; // CRLF CRLF = 4 bytes
    std::string_view body = raw_request.substr(start_pos);
    parse_body(body);
}

void Request::parse_request_line(std::string_view request_line) {
    // method SP request-uri SP http-version

    // get the method
    size_t sp1 = request_line.find(' ', 0);
    if (sp1 == std::string_view::npos) {
        throw std::runtime_error("malformed request-line, method SP not found");
    }
    std::string_view method = request_line.substr(0, sp1);
    if (method == "GET") {
        this->method = HTTPMethod::GET;
    } else if (method == "POST") {
        this->method = HTTPMethod::POST;
    } else {
        this->method = HTTPMethod::UNKNOWN;
    }

    // get the request-uri
    size_t sp2 = request_line.find(' ', sp1 + 1);
    if (sp2 == std::string_view::npos) {
        throw std::runtime_error("malformed request-line, request-uri SP not found");
    }
    this->uri = request_line.substr(sp1 + 1, sp2 - (sp1 + 1));

    // get the http-version
    std::string_view version = request_line.substr(sp2 + 1, request_line.size() - (sp2 + 1));
    if (version != "HTTP/1.1") {
        throw std::runtime_error("wrong HTTP version received");
    }
}

void Request::parse_headers(std::string_view headers) {
    // for each header, they end with CRLF

    size_t start_pos = 0;
    size_t end_pos =  headers.find(CRLF);
    while (start_pos < headers.size()) {
        if (end_pos == std::string_view::npos) {
            throw std::runtime_error("malformed header, CRLF not found");
        }
        std::string_view header = headers.substr(start_pos, end_pos - start_pos);

        // header-name: header-value
        size_t colon_pos = header.find(HEADER_DELIM);
        if (colon_pos == std::string_view::npos) {
            throw std::runtime_error("malformed header, `: ` not found");
        }
        std::string_view header_name = header.substr(start_pos, colon_pos);
        std::string_view header_value = header.substr(colon_pos + 2);
        if (header_name == "Connection") {
            if (header_value == "keep-alive") {
                this->keep_alive = true;
            } else {
                this->keep_alive = false;
            }
        }
        this->headers.emplace(header_name, header_value);

        start_pos = end_pos + 2;
        end_pos = headers.find(CRLF, start_pos);
    }
}

void Request::parse_body(std::string_view body) {
    // TODO: parse JSON for POST requests
    this->body = body;
}
