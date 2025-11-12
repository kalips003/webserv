#include "webserv.hpp"

#include <fstream>

#include <cctype>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////]
bool    parse_config_file(const char* confi_file, server_settings& serv_sett);
bool    parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn);
bool    parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b);
///////////////////////////////////////////////////////////////////////////////]
bool    read_header_first_line(std::string& line, http_request& to_fill);
///////////////////////////////////////////////////////////////////////////////]


///////////////////////////////////////////////////////////////////////////////]
//                                 CONFIG FILE
///////////////////////////////////////////////////////////////////////////////]
// MAIN function parsing the infile.conf
// reads the full file, filling the struct server_config
// return false if any parsing error
// DOES NOT check for the validity of the settings 
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
// read: "server_name myserver.local"
// return true if pair is correct
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
// reads: "location /images {
//                  root /var/www/images; hello cat
//                  cgi /usr/bin/python3} a"
bool    parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b) {

    std::string s = line.substr(0, pos);

// extract name & arg
    std::pair<std::string, std::string> kv;
    if (!parse_key_value(s, kv)) {
        std::cerr << ERR0 RED "invalid kv value: [" RESET << s << RED "]" RESET << std::endl;
        return false;
    }
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
        if (s2.empty()) return true;
        if (s2.find('}') != std::string::npos || !parse_key_value(s2, kv)) {
            std::cerr << ERR0 RED "invalid block: [" RESET << s << RED "]" RESET << std::endl;
            return false;
        }
        b.settings[kv.first] = kv.second;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////]
//                                 HTTP REQUEST
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
// http request line
// METHOD PATH VERSION
bool    read_header_first_line(std::string& line, http_request& to_fill) {
    
    std::vector<std::string> v;
    std::stringstream ss(line);
    std::string s;

    if (!(ss >> to_fill.method >> to_fill.path >> to_fill.version))

    while (ss >> s) v.push_back(s);
    if (v.size() != 3) {
        std::cerr << RED "bad http request line" RESET << std::endl;
        return false;
    }
    to_fill.method = v[0];
    to_fill.path = v[1];
    to_fill.version = v[2];

    return true;
}
