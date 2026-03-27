#include "../hpps/string_utils.hpp"
#include <regex>

namespace util {
    static const std::regex INVALID_FS_CHARS(R"([<>:"/\\|?*])");
    std::string safe_name(const std::string& input, const std::string& fallback) {
        if (input.empty()) return fallback;
        std::string name = std::regex_replace(input, INVALID_FS_CHARS, "_");
        while (!name.empty() && (name.back() == '.' || name.back() == ' '))
            name.pop_back();
        return name.empty() ? fallback : name;
    }
}