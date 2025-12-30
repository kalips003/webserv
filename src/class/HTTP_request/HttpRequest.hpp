#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

#include <defines.hpp>

///////////////////////////////////////////////////////////////////////////////]
// "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n<body>"
///////////////////////////////////////////////////////////////////////////////]
/**
 * Represents a Request HTTP
 *
 * Owns all the parsings tools
 */
class HttpRequest {

private:
///////////////////////////////////////////////////////////////////////////////]
    std::string method; // GET
    std::string path; // /index.html
    std::string version; // HTTP/1.1

    std::map<std::string, std::string>  headers; // Accept: html

    std::string body; // buffer for the body
    int         fd_body;
    size_t      body_bytes_received;
    ssize_t     body_size;
    enum BodyMode   body_type; // content-length mode; chunked-transfer mode; no body expected (GET, HEAD); multipart (POST form/file)

    int         header_delim_progress; // where are we of "\r\n\r\n"
///////////////////////////////////////////////////////////////////////////////]

public:
    HttpRequest() : fd_body(-1), body_bytes_received(0), body_size(0),
        body_type(BODY_NONE), header_delim_progress(0) {}
    ~HttpRequest() { if (fd_body > 0) close(fd_body); }

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

std::ostream& operator<<(std::ostream& os, HttpRequest& r);

#endif