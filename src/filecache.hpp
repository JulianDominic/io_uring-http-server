#pragma once
#include <string_view>
#include <string>
#include <unordered_map>

#define URI_DIR "public"

struct string_hash {
    using is_transparent = void; // helps me make lookups without constructing a string
    size_t operator()(std::string_view s) const {
        return std::hash<std::string_view>{}(s);
    }
    size_t operator()(const std::string &s) const {
        return std::hash<std::string>{}(s);
    }
};

class CachedFile {
public:
    std::string body;
    std::string content_type;
    int content_length;
};

class FileCache {
    std::unordered_map<std::string, CachedFile, string_hash, std::equal_to<>> cache;
public:
    bool contains(std::string_view);
    const CachedFile& get(std::string_view);
    void build();
};
