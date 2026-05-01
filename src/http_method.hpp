#pragma once
#include <ostream>
#include <string>
#include <unordered_map>

enum class HTTPMethod {
    UNKNOWN,
    GET,
    POST,
};

inline std::unordered_map<std::string, HTTPMethod> method_str_to_enum{
    { "GET", HTTPMethod::GET },
    { "POST", HTTPMethod::POST },
};

inline std::ostream& operator<<(std::ostream& os, const HTTPMethod& method) {
    switch (method) {
        case HTTPMethod::GET: return os << "GET";
        case HTTPMethod::POST: return os << "POST";
        default: return os << "UNKNOWN";
    }
}
