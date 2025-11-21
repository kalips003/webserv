#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <string>
#include <iostream>

#include "_colors.h"

#include "defines.hpp"
#include "structs.hpp"

#include "Task.hpp"
#include "connection.hpp"
#include "Server.hpp"
///////////////////////////////////////////////////////////////////////////////]
// A
Task* createTask(const std::string& method, http_request& req, http_answer& ans);
bool    set_flags(int fd_to_set, int flag_to_add);
///////////////////////////////////////////////////////////////////////////////]
// D    Dico
const std::map<int, std::string>&   http_status_map();
std::string   return_http_from_code(int code);
const std::vector<std::string>& http_method_list();
HttpMethod isMethodValid(const std::string& s);
const std::vector<std::string>& HTTP_Request_Headers_list();
int isHTTP_Request_HeadersValid(const std::string& s);
const std::vector<std::string>& HTTP_Response_Headers_list();

//  TOOLS
///////////////////////////////////////////////////////////////////////////////]
//  PARSING
bool    atoi_v2(const std::string& input, int& rtrn);
bool printErr(const char* errmsg);
std::string trim_white(const std::string& s);
std::string trim_any(const std::string& s, const char *to_trim);
std::vector<std::string> splitOnDelimitor(const std::string& s, std::string delimit);
std::vector<std::string> splitOnWhite(const std::string& s);
//  OTHERS
bool    parse_config_file(const char* confi_file, server_settings& serv_sett);
bool    parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn);
bool    parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b);
bool    read_header_first_line(std::string& line, http_request& to_fill);
std::string itostr(int n);


///////////////////////////////////////////////////////////////////////////////]
template <typename Num>
std::string itostd(Num n) {

    std::ostringstream oss;
    oss << n;
    return oss.str();
}

#endif