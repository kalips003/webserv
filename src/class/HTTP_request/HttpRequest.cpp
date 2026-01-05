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
/** Parse given string, add it to _buffer
* if delim ("\r\n") not found, return FIRST
* if delim found, parse the HEAD of request, return READING_HEADER if valid
* return err code if error
*/
int    HttpRequest::parse_header_first_read(std::string& str_buff) {

    _buffer += str_buff;

    if (_buffer.size() > MAX_LIMIT_FOR_HEAD)
        return 400;

    size_t  pos = _buffer.find("\r\n");
    if (pos == std::string::npos)
        return FIRST;

    std::string head = _buffer.substr(0, pos);
    _buffer = _buffer.substr(pos + 2);

    std::stringstream ss(head);
    std::string word;

    if (!(ss >> word) || isMethodValid(word) < 0)
        return 501; // or 400
    _method = word;

    if (!(ss >> word) || !isPathValid(word))
        return 400;
    _path = word;

    if (!(ss >> word) || word != "HTTP/1.1")
        return 505;
    _version = word;

    if (ss >> word) // extra garbage after the 3 tokens
        return 400;

    return READING_HEADER;
}

void    HttpRequest::addBuffer(std::string& s) {

    _buffer += s;
}

/** Parse given string for end of headers delim ("\r\n\r\n") */
int    HttpRequest::check_buffer_for_rnrn(std::string& buff) {

    std::string delim = "\r\n\r\n";

    size_t  pos = buff.find(delim[_header_delim_progress]);

    for (size_t i = _header_delim_progress; i < delim.size(); ++i) {
    
        if ()
    }

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

int    HttpRequest::check_buffer_for_rnrn_v2(std::string& buff) {

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



void    HttpRequest::f1(std::string& str_buff) {

    if (_body_size < 4096)
        _body += str_buff;
    else if (_fd_body >= 0)
        write(_fd_body, buff, bytes_recv); // write(fd_body, buff, bytes)
    _body_bytes_received += bytes_recv;
    if (_body_bytes_received >= static_cast<size_t>(_body_size))
        return DOING;

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



/** Check the syntax validity of the path, doesnt check if file exist */
bool	isPathValid(std::string& path) {

	if (path.empty())
		return false;
	if (path[0] != '/')
		return false;
	for (size_t i = 0; i < path.size(); ++i) {
		unsigned char c = path[i];
		if (std::iscntrl(c) || c == ' ')
			return false;
	}
	if (path.find("..") != std::string::npos)
		return false;
	return true;
}