#pragma once
#include <string_view>
#include <string>
#include <unordered_map>

#define URI_DIR "public"

class CachedFile {
public:
    std::string body;
    std::string content_type;
    int content_length;
};

class FileCache {
    std::unordered_map<std::string, CachedFile> cache;
public:
    bool contains(std::string_view);
    const CachedFile& get(std::string_view);
    void build();
};
