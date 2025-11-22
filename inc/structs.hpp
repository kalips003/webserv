#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <string>
#include <map>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>

///////////////////////////////////////////////////////////////////////////////]
// name arg { set1 a; set2 b }
struct block {
    std::string                         name;
    std::string                         arg;
    std::map<std::string, std::string>  settings;
};

// name arg
// name arg { set1 a; set2 b }
struct server_settings {
    std::map<std::string, std::string>  global_settings;
    std::vector<block>                  block_settings;
    int                                 port_num;

    server_settings() : port_num(-1) {}
    std::string find_setting(const std::string& set) const {

        map_strstr::const_iterator it = global_settings.begin();
        it = global_settings.find(set);
        if (it == global_settings.end()) {
            std::cerr << RED "setting not found: " RESET << set << std::endl;
            return "";
        }
        else 
            return it->second;
    }
    std::string find_setting_inBlock(const std::string& set) {
        return "";
    }
};

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
// GET /index.html HTTP/1.1\r\n
// Host: example.com\r\n
// User-Agent: curl/7.81.0\r\n
// Accept: */*\r\n
// \r\n
// <body content if any...>
enum BodyMode {
    BODY_NONE,
    BODY_CONTENT_LENGTH,
    BODY_CHUNKED
};

#include <sstream>
#include <unistd.h>
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
// "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n<body>"
struct http_request {

    std::string method; // GET
    std::string path; // /index.html
    std::string version; // HTTP/1.1

    std::map<std::string, std::string>  headers;

    std::string body;
    int         fd_body;
    size_t      body_bytes_received;
    ssize_t     body_size;
    enum BodyMode   body_type; // content-length mode; chunked-transfer mode; no body expected (GET, HEAD); multipart (POST form/file)

    int         header_delim_progress; // where are we of "\r\n\r\n"

    http_request() : fd_body(-1), body_bytes_received(0), body_size(0),
        body_type(BODY_NONE), header_delim_progress(0) {}
    ~http_request() { if (fd_body > 0) close(fd_body); }

/*  parse the headers to find if there is body, return its length */
    ssize_t      isThereBody();
    std::string find_setting(const std::string& set) const {

        map_strstr::const_iterator it = headers.begin();
        it = headers.find(set);
        if (it == headers.end()) {
            std::cerr << RED "setting not found: " RESET << set << std::endl;
            return "";
        }
        else 
            return it->second;    
    }
};

std::ostream& operator<<(std::ostream& os, http_request& r);
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
// HTTP/1.1 200 OK
struct http_answer {

    std::string version; // HTTP/1.1
    int         status; // 200
    std::string msg_status; // OK

    std::map<std::string, std::string>  _headers;

    std::string     head; // 1) <body>, after ini(): head<body>
    std::string     body_leftover;
    int             fd_body;

    size_t          _full_size;
    size_t          _bytes_sent;


    http_answer() : version("HTTP/1.1"), status(200), msg_status("OK"), 
        fd_body(-1), _full_size(0), _bytes_sent(0) {}
    ~http_answer() {
        if (fd_body >= 0) close(fd_body);
    }

/*  take the filled answer, concatenate headers into head */
    void http_answer_ini();
};

#endif