#include "filecache.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>

bool FileCache::contains(std::string_view uri) {
    return this->cache.contains(std::string(uri));
}

const CachedFile& FileCache::get(std::string_view  uri) {
    return this->cache[std::string(uri)];
}

std::string get_content_type(std::string filepath) {
    if (filepath.ends_with("html")) { return "text/html"; }
    else if (filepath.ends_with("ico")) { return "image/vnd.microsoft.icon"; } // from mozilla mime types
    else if (filepath.ends_with("json")) { return "application/json"; }
    else { return ""; }
}

void FileCache::build() {
    // walk through the directory
    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(URI_DIR)) {
        // read the file
        CachedFile cf;
        cf.content_length = dir_entry.file_size();
        cf.content_type = get_content_type(dir_entry.path());
        if (cf.content_type == "") {
            throw std::runtime_error("unsupported file type found");
        }
        cf.body = std::string(cf.content_length, '\0');
        std::ifstream input(dir_entry.path());
        input.read(&cf.body[0], cf.content_length);

        // load resources into the cache
        this->cache.emplace(dir_entry.path().string().substr(strlen(URI_DIR)), cf);
    }
}
