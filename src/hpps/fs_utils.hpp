#pragma once
#include <filesystem>
#include <string>

namespace util {
    void write_text(const std::filesystem::path& path, const std::string& text);
}