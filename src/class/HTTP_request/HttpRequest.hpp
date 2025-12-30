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
    std::string         _method; // GET
    std::string         _path; // /index.html
    std::string         _version; // HTTP/1.1

    map_strstr          _headers; // Accept: html

    std::string         _body; // buffer for the body
    int                 _fd_body;
    size_t              _body_bytes_received;
    ssize_t             _body_size;
    enum BodyMode       _body_type; // content-length mode; chunked-transfer mode; no body expected (GET, HEAD); multipart (POST form/file)

    int                 _header_delim_progress; // where are we of "\r\n\r\n"
///////////////////////////////////////////////////////////////////////////////]

public:
    HttpRequest() : _fd_body(-1), _body_bytes_received(0), _body_size(0),
        _body_type(BODY_NONE), _header_delim_progress(0) {}
    ~HttpRequest() { if (_fd_body > 0) close(_fd_body); }

/*  parse the headers to find if there is body, return its length */
    ssize_t      isThereBody();
    std::string find_setting(const std::string& set) const ;





    enum ConnectionStatus ft_read(char *buff, size_t sizeofbuff, int client_fd);
};

std::ostream& operator<<(std::ostream& os, HttpRequest& r);

#endif