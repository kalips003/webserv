#include "HttpRequest.hpp"

#include "_colors.h"
#include <iostream>

///////////////////////////////////////////////////////////////////////////////]
#include <unistd.h>
HttpRequest::~HttpRequest() { if (_fd_body > 0) close(_fd_body); }

///////////////////////////////////////////////////////////////////////////////]
std::string HttpRequest::find_setting(const std::string& set) const {

    map_strstr::const_iterator it = _headers.begin();
    it = _headers.find(set);
    if (it == _headers.end()) {
        std::cerr << RED "setting not found: " RESET << set << std::endl;
        return "";
    }
    else 
        return it->second;    
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
#include "HttpMethods.hpp"
#include <sstream>
///////////////////////////////////////////////////////////////////////////////]
int    HttpRequest::parse_header_first_read() {

    if (_buffer.find("\r\n") == std::string::npos)
        return FIRST;

    std::stringstream ss(_buffer);
    std::string word;

    ss >> word;
    
    if (isMethodValid(word) < 0)
        return 405;
    if (!(ss >> word >> word) || word != "HTTP/1.1")
        return 400;
    return READING_HEADER;
}

void    HttpRequest::addBuffer(std::string& s) {

    _buffer += s;
}

int    HttpRequest::check_buffer_for_rnrn(std::string& buff) {

    std::string delim = "\r\n\r\n";

    while (*buff != '\0') {

        if (*buff == delim[_request._header_delim_progress]) {
            _request._header_delim_progress++;

            if (_request._header_delim_progress == delim.size()) {
                _buffer.push_back(*buff);
                buff++;
                return parse_header_wrapper(buff);
            }
        }
        else
            _request._header_delim_progress = (*buff == delim[0]);
        
        _buffer.push_back(*buff);
        buff++;
    }
    return static_cast<ConnectionStatus>(_status);
}





///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
#include "Tools1.hpp"
ssize_t      HttpRequest::isThereBody() {

    std::map<std::string, std::string>::iterator it = _headers.find("body-size");
    if (it == _headers.end())
        return 0;
    int r;
    if (!atoi_v2(it->second, r) || r < 0)
        return -1;
    return static_cast<ssize_t>(r);
}

std::ostream& operator<<(std::ostream& os, HttpRequest& r) {

    os << C_542 "---------------------------------------------\n" RESET;
    os << C_542 "\tREQUEST:\n" RESET;
    os << r.getMethod() << " " << r.getPath() << " " << r.getVersion() << std::endl;
    os << C_542 "\tHEADERS:\n" RESET;
    for (map_strstr::const_iterator  h = r.getHeaders().begin(); h != r.getHeaders().end(); h++) {
        os << C_114 << h->first << RESET ": " << h->second << std::endl;
    }

    os << C_542 "\tBODY:\n[" RESET << r.getBody() << C_542 "]" RESET << std::endl;
    os << C_542 "fd_body: " RESET << r.getFdBody() << std::endl;
    os << C_542 "body_bytes_received: " RESET << r.getBodyBytesReceived() << std::endl;
    os << C_542 "body_size: " RESET << r.getBodySize() << std::endl;         
    os << C_542 "---------------------------------------------\n" RESET;
    return os;       
}
