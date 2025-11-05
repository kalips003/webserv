#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <string>
#include <map>
#include <vector>

///////////////////////////////////////////////////////////////////////////////]
// name arg { set1 a; set2 b }
struct block {
    std::string                         name;
    std::string                         arg;
    std::map<std::string, std::string>  settings;
};

struct server_settings {
    std::map<std::string, std::string>  global_settings;
    std::vector<block>                  block_settings;
    int                                 port_num;

    server_settings() : port_num(-1) {}
};

///////////////////////////////////////////////////////////////////////////////]
struct http_request {

    std::string method;
    std::string path;
    std::string version;

    std::map<std::string, std::string>  headers;

    std::string body;


// GET /index.html HTTP/1.1\r\n
// Host: example.com\r\n
// User-Agent: curl/7.81.0\r\n
// Accept: */*\r\n
// \r\n
// <body content if any...>

    bool parse_header(const std::string& h) {

    }
};

#define RECEVING_HEADER 0
#define RECEVING_BODY 1
struct http_request_wrapper {

    http_request    request;

    std::string     msg;
// msg.reserve(8192); // optional

    struct sockaddr_in  client_addr;
    socklen_t           addr_len;
    int                 client_fd;

    ssize_t             bytes_received;
    int                 delim_status;
    int                 status;

    http_request_wrapper() :
        client_addr(), addr_len(sizeof(client_addr)), 
        client_fd(-1), bytes_received(0), delim_status(0), status(0) {}
};

#endif