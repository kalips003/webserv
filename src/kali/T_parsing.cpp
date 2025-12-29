#include "webserv.hpp"

#include <fstream>

#include <cctype>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////]
bool    parse_config_file(const char* confi_file, ServerSettings& serv_sett);
bool    parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn);
bool    parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b);
///////////////////////////////////////////////////////////////////////////////]
bool    read_header_first_line(std::string& line, http_request& to_fill);
///////////////////////////////////////////////////////////////////////////////]




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
