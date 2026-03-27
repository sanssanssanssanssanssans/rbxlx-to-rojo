#include "../hpps/rbx_to_lua.hpp"
#include "../hpps/simple_xml.hpp"
#include "../hpps/string_utils.hpp"
#include "../hpps/fs_utils.hpp"
#include <fstream>
#include <algorithm>
#include <optional>
using namespace rbx;
namespace fs = std::filesystem;
static bool remove_empty_dirs(const std::filesystem::path& dir) {
    bool is_empty = true;
    for (auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_directory()) {
            if (!remove_empty_dirs(entry.path())) {
                is_empty = false;
            }
        } else {
            is_empty = false;
        }
    }
    if (is_empty) {
        std::filesystem::remove(dir);
        return true;
    }
    return false;
}
namespace rbx {
    static std::optional<std::string> get_prop(XMLNode* props, const std::string& key) {
        for (auto c : props->children) {
            if (c->attrs["name"] == key)
                return c->text;
        }
        return std::nullopt;
    }
    static InstanceNode parse_item(XMLNode* item) {
        InstanceNode node;
        node.class_name = item->attrs["class"];
        if (node.class_name.empty()) node.class_name = "Instance";
        auto props = item->find_child("Properties");
        std::optional<std::string> name;
        std::optional<std::string> source;
        if (props) {
            name = get_prop(props, "Name");
            if (node.class_name == "Script" ||
                node.class_name == "LocalScript" ||
                node.class_name == "ModuleScript") {
                source = get_prop(props, "Source");
            }
        }
        node.name = util::safe_name(name.value_or(node.class_name));
        if (source) node.source = source;
        for (auto child : item->find_all("Item"))
            node.children.push_back(parse_item(child));
        return node;
    }
    InstanceNode load_rbx_xml(const fs::path& path) {
        std::ifstream in(path);
        std::string content((std::istreambuf_iterator<char>(in)), {});
        XMLNode* root = parse_xml(content);
        InstanceNode synthetic;
        synthetic.class_name = "DataModel";
        synthetic.name = util::safe_name(path.stem().string(), "Game");
        for (auto item : root->find_all("Item"))
            synthetic.children.push_back(parse_item(item));
        return synthetic;
    }
    static std::string script_filename(const InstanceNode& node) {
        std::string base = util::safe_name(node.name, node.class_name);
        if (node.class_name == "Script") return base + ".server.lua";
        if (node.class_name == "LocalScript") return base + ".client.lua";
        if (node.class_name == "ModuleScript") return base + ".lua";
        return base + ".lua";
    }
    static void emit_node(const InstanceNode& node, const fs::path& dir) {
        if (node.class_name == "Script" ||
            node.class_name == "LocalScript" ||
            node.class_name == "ModuleScript") {
            util::write_text(dir / script_filename(node), node.source.value_or(""));
            return;
        }
        fs::path next = dir / util::safe_name(node.name, node.class_name);
        fs::create_directories(next);
        for (auto& c : node.children)
            emit_node(c, next);
    }
    fs::path export_to_lua_project(const InstanceNode& root, const fs::path& out) {
        fs::create_directories(out / "src");
        for (auto& c : root.children)
            emit_node(c, out / "src");
        remove_empty_dirs(out / "src");
        util::write_text(out / "default.project.json",
            "{\n  \"name\": \"" + root.name + "\",\n"
            "  \"tree\": {\"$className\": \"DataModel\", \"$path\": \"src\"}\n}");
        return out;
    }
    std::string detect_rbx_format(const fs::path& path) {
        std::ifstream in(path, std::ios::binary);
        std::string head(512, '\0');
        in.read(head.data(), head.size());
        std::string s = head;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s.find("<roblox!") == 0) return "binary";
        if (s.find("<?xml") == 0 || s.find("<roblox") == 0) return "xml";
        return "unknown";
    }
}