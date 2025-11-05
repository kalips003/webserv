#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <string>
#include <iostream>

#include "_colors.h"

#include "server.hpp"

///////////////////////////////////////////////////////////////////////////////]
//  TOOL PARSING
///////////////////////////////////////////////////////////////////////////////]
std::string trim_white(const std::string& s);
bool    parse_config_file(const char* confi_file, server_settings& serv_sett);
bool    parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn);
bool    parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b);

///////////////////////////////////////////////////////////////////////////////]
//  TOOL OTHER
///////////////////////////////////////////////////////////////////////////////]
bool    atoi_v2(std::string& input, int& rtrn);
bool    printErr(const char* errmsg);

#endif