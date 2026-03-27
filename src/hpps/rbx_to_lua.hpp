#pragma once
#include <string>
#include <vector>
#include <optional>
#include <filesystem>
namespace rbx {
    struct InstanceNode {
        std::string class_name;
        std::string name;
        std::optional<std::string> source;
        std::vector<InstanceNode> children;
    };
    InstanceNode load_rbx_xml(const std::filesystem::path& path);
    std::filesystem::path export_to_lua_project(
        const InstanceNode& root,
        const std::filesystem::path& output_dir
    );
    std::string detect_rbx_format(const std::filesystem::path& path);
}