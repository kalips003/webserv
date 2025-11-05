#include <iostream>
#include <fstream>

#include <cctype>
#include <sstream>

#include "structs.hpp"
#include "_colors.h"

///////////////////////////////////////////////////////////////////////////////]
std::string trim_white(const std::string& s);
bool    parse_config_file(const char* confi_file, server_settings& serv_sett);
bool    parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn);
bool    parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b);
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::string trim_white(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return ""; // all whitespace
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

///////////////////////////////////////////////////////////////////////////////]
bool    parse_config_file(const char* confi_file, server_settings& serv_sett) {

    std::string s(confi_file);
    if (s.size() < 5 || s.substr(s.size() - 5) != ".conf") {
        std::cerr << ERR "one .conf arg is required" RESET << std::endl;
        return false;
    }

    std::ifstream   file(confi_file);
    if (!file.is_open()) {
        std::cerr << ERR "cant open file: " << C_420 << confi_file << RESET << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim_white(line);
        if (line.empty() || line[0] == '#')
            continue;
        size_t pos = line.find('{');
        if (pos != std::string::npos) {
            block   b;
            if (!parse_blocks(file, line, pos, b))
                return false;
            serv_sett.block_settings.push_back(b);
        }
        else {
            std::pair<std::string, std::string> kv;
            if (!parse_key_value(line, kv))
                return false;
            serv_sett.global_settings[kv.first] = kv.second;
        }
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////]
bool    parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn) {

    std::stringstream           ss(trim_white(line));
    std::vector<std::string>    tokens;
    std::string                 tok;

    while (ss >> tok)
        tokens.push_back(tok);

    if (tokens.size() != 2) {
        std::cerr << ERR0 RED "invalid line: [" RESET << line << RED "]" RESET << std::endl;
        return false;
    }
    rtrn = std::make_pair(tokens[0], tokens[1]);

    return true;
}

///////////////////////////////////////////////////////////////////////////////]
bool    parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b) {

    std::string s = line.substr(0, pos);

// extract name & arg
    std::pair<std::string, std::string> kv;
    if (!parse_key_value(s, kv))
        return false;
    b.name = kv.first;
    b.arg = kv.second;

// concatenate all block:
// s = "{\nroot /var/www/images; hello cat\n cgi /usr/bin/python3\n}"
    s = line.substr(pos + 1);
    std::string s2;
    while (std::getline(file, s2)) {
        s += "\n" + s2;
        if (s2.find('}') != std::string::npos)
            break;
    }
// replace ';' with '\n'
    for (size_t i = 0; i < s.size(); i++)
        if (s[i] == ';') s[i] = '\n';

// check validity of block, fills settings
    std::stringstream ss(s);
    while (getline(ss, s2)) {
        s2 = trim_white(s2);
        if (s2.empty()) continue;
        if (s2.find('}') != std::string::npos)
            s2 = trim_white(s2.substr(0, s2.size() - 1));
        if (s2.find('}') != std::string::npos || !parse_key_value(s2, kv))
            return false;
        b.settings[kv.first] = kv.second;
    }

    return true;
}