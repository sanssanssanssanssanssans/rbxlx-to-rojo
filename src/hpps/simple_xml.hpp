#pragma once
#include <string>
#include <vector>
#include <map>

struct XMLNode {
    std::string name;
    std::map<std::string, std::string> attrs;
    std::string text;
    std::vector<XMLNode*> children;
    XMLNode* find_child(const std::string& n);
    std::vector<XMLNode*> find_all(const std::string& n);
};

XMLNode* parse_xml(const std::string& content);