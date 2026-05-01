#pragma once
#include <string>
#include <unordered_map>

enum class HTTPStatusCode {
    OK = 200,
    CREATED = 201,
    NOT_FOUND = 404,
};

inline std::unordered_map<HTTPStatusCode, std::string> status_code_to_string{
    { HTTPStatusCode::OK, "OK" },
    { HTTPStatusCode::CREATED, "Created" },
    { HTTPStatusCode::NOT_FOUND, "Not Found" },
};
