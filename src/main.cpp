#include "hpps/rbx_to_lua.hpp"
#include <iostream>
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: rbx-to-lua input [-o out]\n";
        return 1;
    }
    std::string input = argv[1];
    std::string output = "out";
    if (argc >= 4 && std::string(argv[2]) == "-o")
        output = argv[3];
    auto fmt = rbx::detect_rbx_format(input);
    if (fmt != "xml") {
        std::cerr << "XML만 지원\n";
        return 2;
    }
    auto root = rbx::load_rbx_xml(input);
    rbx::export_to_lua_project(root, output);
    std::cout << "완료\n";
}