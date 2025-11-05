#include <iostream>
#include <string> 
#include <map>
#include <sstream>

struct http_request {

    std::string method;
    std::string path;
    std::string version;

    std::map<std::string, std::string>  headers;

    std::string body;
};

void    parse_request(std::string& request_str) {

    size_t  pos;
    pos = request_str.find("\r\n");
    std::stringstream ss;
    ss << request_str.substr(0, pos);

    http_request request;

    ss >> request.method;
    ss >> request.path;
    ss >> request.version;

}