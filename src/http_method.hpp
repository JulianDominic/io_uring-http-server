#pragma once
#include <string>
#include <unordered_map>

enum class HTTPMethod {
    UNKNOWN,
    GET,
};

inline std::unordered_map<std::string, HTTPMethod> method_str_to_enum{
    { "GET", HTTPMethod::GET },
};
