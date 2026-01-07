#include "HttpRequest.hpp"
#include "_colors.h"

#include <iostream>
#include <unistd.h>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <fcntl.h>

#include "Tools1.hpp"
#include "HttpMethods.hpp"
///////////////////////////////////////////////////////////////////////////////]
// #include <unistd.h>
HttpRequest::~HttpRequest() { if (_fd_body > 0) close(_fd_body); }

// #include "Tools1.hpp" // atoi
///////////////////////////////////////////////////////////////////////////////]
/** parse the headers to find if there is a body, return its length
*
* @return length of "body-size"
*
* if no "body-size" is found, return 0,
* if "body-size" is incorrect, return -1			---*/
ssize_t      HttpRequest::isThereBody() const {

    map_istr::const_iterator it = _headers.find("body-size");
    if (it == _headers.end())
        return 0;
    int r;
    if (!atoi_v2(it->second, r) || r < 0)
        return -1;
    return static_cast<ssize_t>(r);
}

///////////////////////////////////////////////////////////////////////////////]
/** find the given setting in the _headers
*	@return the string value of the setting
*
* if setting not found, return "" empty string	---*/
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

// #include "HttpMethods.hpp"
// #include <sstream>
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
/** Add given string to _buffer, parse it for "\r\n"
* 
* if found, parse the HEAD of request, and remove it from _buffer
* 
* @return if delim found, return READING_HEADER if HEAD valid, errCode if not
* 
* if delim ("\r\n") not found, return FIRST			---*/
int    HttpRequest::readingFirstLine(std::string& str_buff) {

    _buffer += str_buff;

    if (_buffer.size() > MAX_LIMIT_FOR_HEAD) {
		std::cerr << RED "Limit max reached before finding CRLF" RESET << std::endl;
        return 400;
	}

    size_t  pos = _buffer.find("\r\n");
    if (pos == std::string::npos)
        return FIRST;

    std::string head = _buffer.substr(0, pos); // "GET /index.html HTTP/1.1"
    // _buffer = _buffer.substr(pos + 2); // "Host: ex..."

    std::stringstream ss(head);
    std::string word;

    if (!(ss >> word) || isMethodValid(word) < 0) {
		std::cerr << RED "Invalid Method: " RESET << word << std::endl;
        return 501; // or 400
	}
    _method = word;

    if (!(ss >> word) || !isPathValid(word)) {
		std::cerr << RED "Invalid Path: " RESET << word << std::endl;
        return 400;
	}
    _path = word;

    if (!(ss >> word) || word != "HTTP/1.1") {
		std::cerr << RED "Invalid Version: " RESET << word << std::endl;
        return 505;
	}
    _version = word;

    if (ss >> word) { // extra garbage after the 3 tokens
		std::cerr << RED "Invalid HEAD: " RESET << _buffer.substr(0, _buffer.find("\r\n")) << std::endl;
        return 400;
	}

    return READING_HEADER;
}

//-----------------------------------------------------------------------------]
/** Check the syntax validity of the path, doesnt check if file exist	---*/
bool	HttpRequest::isPathValid(std::string& path) {

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

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
/** Called while reading headers, 
check _buffer + buff for CRLF
*
* @return READING_HEADER if not found
*
* DOING if no body
*
* READING_BODY if body and parsing went well
* 
* errCode if parsing went bad			---*/
int    HttpRequest::readingHeaders(std::string& buff) {

    std::string delim = "\r\n\r\n";

	size_t n = _buffer.size() - (_buffer.size() < 3 ? _buffer.size() : 3);
	std::string new_buff = _buffer.substr(n) + buff;

    size_t  pos = new_buff.find(delim);

	_buffer += buff;
	if (pos == std::string::npos)
		return READING_HEADER;
	else
		return parsingHeaders(delim);
}

//-----------------------------------------------------------------------------]
/** Wrapper for all the parsing and checking
* @return READING_BODY if body, DOING if not, errCode if error 	---*/
int	HttpRequest::parsingHeaders(std::string& delim) {

// Move the start of the body into _body
	size_t pos = _buffer.find(delim);
	_body = _buffer.substr(pos + delim.size());
	_buffer = _buffer.substr(0, pos + 2); // leave one '\r\n' after the last header
	_body_bytes_received += _body.size();

	int errRtrn = parse_header_for_syntax();
	if (errRtrn != READING_BODY) {
		std::cerr << RED "ERROR SYNTAX - " RESET "while parsing headers" << std::endl;
		return errRtrn;
	}

	errRtrn = parse_headers_for_validity();
	if (errRtrn > 100) {
		std::cerr << RED "ERROR - " RESET "while parsing headers" << std::endl;
		return errRtrn;
	}

	return errRtrn;
}

// #include <algorithm>
//-----------------------------------------------------------------------------]
/** Parse the _buffer containing CRLF,
* split it into _headers
*
* Parse only invalid syntax, not validity of the content
*
* @return READING_BODY if parsing was succesful, errCode otherwise
*
* clear _buffer memory after parsing			---*/
int HttpRequest::parse_header_for_syntax() {

    std::vector<std::string> v;
    v = splitOnDelimitor(_buffer, "\r\n");
    if (!v.size()) {
		_buffer.clear();
		printErr(ERR9 "emtpy vector (should never see this)");
        return 400;
	}

    std::vector<std::string>::iterator it = v.begin();
    ++it; // move past first line: "GET /index.html HTTP/1.1"

    while (it != v.end()) {

        size_t colon_pos = it->find(':');
        if (colon_pos == std::string::npos) {
			std::cerr << ERR8 "bad header: " RESET << *it << std::endl;
			_buffer.clear();
            return 400;
		}
            
        std::string key = trim_white(it->substr(0, colon_pos));
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        std::string value = trim_white(it->substr(colon_pos + 1));
        _headers[key] = value;

        ++it;
    }
	_buffer.clear();
    return READING_BODY;
}

//-----------------------------------------------------------------------------]
/** Parse the _headers,
* fill _body_size
*
* @return READING_BODY if body, DOING if not, errCode if error 	---*/
int    HttpRequest::parse_headers_for_validity() {

	_body_size = isThereBody();
	if (_body_size < 0)
		return printErr(ERR9 "bad body-size"), 400;

	if (_body_size > MAX_BODY_SIZE) { // <-----------------------------------------------------]
		openFdBody("./temp/todolater");
	}

// *** OTHER CHECKS AND FILLINGS HERE
	// std::string temp;
	// temp = find_setting("content-length");

	if (!_body_size || _body_bytes_received >= static_cast<size_t>(_body_size))
		return DOING;
	else 
		return READING_BODY;
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
/**	Manage the new read and append it to the correct body 
*
* if body small, append to _body
* if big, write to _fd_body
*
* @return DOING if body finished to be received, READING_BODY otherwise	---*/
int	HttpRequest::readingBody(std::string& buff) {

	if (_body_size < MAX_BODY_SIZE)
		_body += buff;
	else if (_fd_body >= 0)
		write(_fd_body, buff.c_str(), buff.size());

	_body_bytes_received += buff.size();

	if (_body_bytes_received >= static_cast<size_t>(_body_size))
		return DOING;
	return READING_BODY;
}

// #include <sys/stat.h>
// #include <fcntl.h>
///////////////////////////////////////////////////////////////////////////////]
/**	Open the Fd for the big body, 
* copy the remainder of _body into it (and clear it)
*
* @return READING_BODY if all good, errCode otherwise
*
* @flags:
* O_CREAT (if not exist),
* O_RDWR (read/write rights),
* O_TRUNC (safety if name reused),
* 0600 (server-only access)
*	---*/
int	HttpRequest::openFdBody(const char* path) {

	_fd_body = open(path, O_CREAT | O_RDWR | O_TRUNC, 0600);

	if (_fd_body < 0)
		return printErr(ERR9 "bad body-size"), 400;
 
	_tmp_body_path = path;

	write(_fd_body, _body.c_str(), _body.size());

	_body.clear();

	return READING_BODY;
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, HttpRequest& r) {

    os << C_542 "---------------------------------------------\n" RESET;
    os << C_542 "\tREQUEST:\n" RESET;
    os << C_410 << r.getMethod() << " " C_144 << r.getPath() << " " C_232 << r.getVersion() << RESET << std::endl;
    os << C_542 "\tHEADERS:\n" RESET;
    for (map_strstr::const_iterator  h = r.getHeaders().begin(); h != r.getHeaders().end(); h++) {
        os << C_114 << h->first << RESET ": " << h->second << std::endl;
    }

    os << C_542 "\tBODY:\n[" RESET << r.getBody() << C_542 "]" RESET << std::endl;
    os << C_542 "fd_body: " RESET << r.getFdBody() << std::endl;
    os << C_542 "body_size: " RESET << r.getBodySize() << std::endl;         
    os << C_542 "body_bytes_received: " RESET << r.getBodyBytesReceived() << std::endl;
    os << C_542 "---------------------------------------------\n" RESET;
    return os;
}

