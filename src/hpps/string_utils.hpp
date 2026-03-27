#pragma once
#include <string>
namespace util {
    std::string safe_name(const std::string& name, const std::string& fallback = "unnamed");
}   