#include "../hpps/fs_utils.hpp"
#include <fstream>
namespace util {
    void write_text(const std::filesystem::path& path, const std::string& text) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream out(path);
        out << text;
    }
}