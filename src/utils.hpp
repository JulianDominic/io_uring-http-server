#pragma once
#include <vector>
#include <string>
#include <cstring>

inline std::vector<std::string> split(std::string str, const std::string& delim) {
    size_t start_pos = 0;
    std::vector<std::string> result;
    while (true) {
        size_t curr_pos = str.find(delim, start_pos);
        if (curr_pos == std::string::npos) {
            result.push_back(str.substr(start_pos));    
            break;
        }
        result.push_back(str.substr(start_pos, curr_pos - start_pos));
        start_pos = curr_pos + delim.size();
    }
    return result;
}
