#include "../hpps/simple_xml.hpp"

#include <cctype>
#include <stack>
#include <string>
#include <vector>

static inline void ltrim_inplace(std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    s.erase(0, i);
}

static inline void rtrim_inplace(std::string& s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
}

static inline std::string trim(std::string s) {
    ltrim_inplace(s);
    rtrim_inplace(s);
    return s;
}

XMLNode* XMLNode::find_child(const std::string& n) {
    for (auto c : children) {
        if (c && c->name == n) return c;
    }
    return nullptr;
}

std::vector<XMLNode*> XMLNode::find_all(const std::string& n) {
    std::vector<XMLNode*> res;
    for (auto c : children) {
        if (c && c->name == n) res.push_back(c);
    }
    return res;
}

static void skip_ws(const std::string& s, size_t& i) {
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
}

static std::string unescape_xml_text(std::string s) {
    size_t pos = 0;
    while ((pos = s.find("&lt;", pos)) != std::string::npos) s.replace(pos, 4, "<");
    pos = 0;
    while ((pos = s.find("&gt;", pos)) != std::string::npos) s.replace(pos, 4, ">");
    pos = 0;
    while ((pos = s.find("&amp;", pos)) != std::string::npos) s.replace(pos, 5, "&");
    pos = 0;
    while ((pos = s.find("&quot;", pos)) != std::string::npos) s.replace(pos, 6, "\"");
    pos = 0;
    while ((pos = s.find("&apos;", pos)) != std::string::npos) s.replace(pos, 6, "'");
    return s;
}

static void parse_tag_inside(
    const std::string& tag,
    std::string& name,
    std::map<std::string, std::string>& attrs,
    bool& self_closing
) {
    size_t i = 0;
    skip_ws(tag, i);
    while (i < tag.size() && !std::isspace(static_cast<unsigned char>(tag[i])) && tag[i] != '/') {
        name.push_back(tag[i]);
        ++i;
    }
    while (i < tag.size()) {
        skip_ws(tag, i);
        if (i >= tag.size()) break;
        if (tag[i] == '/') {
            self_closing = true;
            break;
        }
        std::string key;
        while (i < tag.size() &&
               tag[i] != '=' &&
               !std::isspace(static_cast<unsigned char>(tag[i]))) {
            key.push_back(tag[i]);
            ++i;
        }
        skip_ws(tag, i);
        if (i >= tag.size() || tag[i] != '=') break;
        ++i;
        skip_ws(tag, i);
        if (i >= tag.size()) break;
        std::string value;
        if (tag[i] == '"' || tag[i] == '\'') {
            char quote = tag[i++];
            while (i < tag.size() && tag[i] != quote) {
                value.push_back(tag[i]);
                ++i;
            }
            if (i < tag.size() && tag[i] == quote) ++i;
        } else {
            while (i < tag.size() &&
                   !std::isspace(static_cast<unsigned char>(tag[i])) &&
                   tag[i] != '/') {
                value.push_back(tag[i]);
                ++i;
            }
        }
        if (!key.empty()) attrs[key] = unescape_xml_text(value);
    }
}

XMLNode* parse_xml(const std::string& s) {
    std::stack<XMLNode*> st;
    XMLNode* root = nullptr;
    size_t i = 0;
    while (i < s.size()) {
        if (s[i] != '<') {
            size_t j = s.find('<', i);
            if (j == std::string::npos) j = s.size();
            if (!st.empty()) {
                std::string text = s.substr(i, j - i);
                text = unescape_xml_text(text);
                st.top()->text += text;
            }
            i = j;
            continue;
        }
        if (i + 1 >= s.size()) break;
        if (s[i + 1] == '?') {
            size_t end = s.find("?>", i + 2);
            if (end == std::string::npos) break;
            i = end + 2;
            continue;
        }
        if (i + 3 < s.size() && s.compare(i, 4, "<!--") == 0) {
            size_t end = s.find("-->", i + 4);
            if (end == std::string::npos) break;
            i = end + 3;
            continue;
        }
        if (i + 8 < s.size() && s.compare(i, 9, "<![CDATA[") == 0) {
            size_t end = s.find("]]>", i + 9);
            if (end == std::string::npos) break;
            if (!st.empty()) {
                st.top()->text += s.substr(i + 9, end - (i + 9));
            }
            i = end + 3;
            continue;
        }
        size_t end = s.find('>', i + 1);
        if (end == std::string::npos) break;
        std::string tag = s.substr(i + 1, end - (i + 1));
        tag = trim(tag);
        if (tag.empty()) {
            i = end + 1;
            continue;
        }
        if (tag[0] == '/') {
            if (!st.empty()) st.pop();
            i = end + 1;
            continue;
        }
        XMLNode* node = new XMLNode();
        bool self_closing = false;
        parse_tag_inside(tag, node->name, node->attrs, self_closing);
        if (node->name.empty()) {
            delete node;
            i = end + 1;
            continue;
        }
        if (!st.empty()) {
            st.top()->children.push_back(node);
        } else {
            root = node;
        }
        if (!self_closing) {
            st.push(node);
        }
        i = end + 1;
    }
    return root;
}