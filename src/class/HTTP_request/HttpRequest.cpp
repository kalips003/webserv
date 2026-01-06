#include "HttpRequest.hpp"
#include "_colors.h"

#include <iostream>
#include <unistd.h>
#include <sstream>

#include "Tools1.hpp"
#include "HttpMethods.hpp"
///////////////////////////////////////////////////////////////////////////////]
// #include <unistd.h>
HttpRequest::~HttpRequest() { if (_fd_body > 0) close(_fd_body); }

///////////////////////////////////////////////////////////////////////////////]
// #include "Tools1.hpp" // atoi
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
// #include "HttpMethods.hpp"
// #include <sstream>
int    HttpRequest::parse_header_first_read(std::string& str_buff) {

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

///////////////////////////////////////////////////////////////////////////////]
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
int    HttpRequest::check_buffer_for_rnrn(std::string& buff) {

    std::string delim = "\r\n\r\n";

	size_t n = _buffer.size() - (_buffer.size() < 3 ? _buffer.size() : 3);
	std::string new_buff = _buffer.substr(n) + buff;

    size_t  pos = new_buff.find(delim);

	_buffer += buff;
	if (pos == std::string::npos)
		return READING_HEADER;
	else {
		pos = _buffer.find(delim);
		_body = _buffer.substr(pos + delim.size());
		_buffer = _buffer.substr(0, pos + 2); // leave one '\r\n' after the last header
		_body_bytes_received += _body.size();
	}
	
    return READING_BODY;
    // return parse_header_wrapper(buff);
}


#include <algorithm>
//?????????????????????????????????????????????????????????????????????????????]
// takes the string containing all the headers, ending in "\r\n\r\n"
// fills the struct http_request, return false on error (parsing errors, invalid http request)
//  parse only invalid syntax, not validity of the content
// bool HttpRequest::parse_header_for_syntax() {

//     std::vector<std::string> v;
//     v = splitOnDelimitor(_buffer, "\r\n");
//     if (!v.size())
//         return printErr(ERR9 "emtpy vector"); // emtpy vector, no headers

// 	v.erase(v.begin());
//     std::vector<std::string>::iterator it = v.begin();
//     ++it;
//     int i = 0;
//     while (!(*it).empty()) {
//         i++;
//         size_t colon_pos = it->find(':');
//         if (colon_pos == std::string::npos)
//             return printErr(ERR8 "bad header?, no colon");           
            
//         std::string key = trim_white(it->substr(0, colon_pos));
//         std::transform(key.begin(), key.end(), key.begin(), ::tolower);
//         std::string value = trim_white(it->substr(colon_pos + 1));
//         _headers[key] = value;

//         ++it;
//     }
//     return true;
// }








// //?????????????????????????????????????????????????????????????????????????????]
// enum ConnectionStatus    HttpRequest::parse_header_wrapper(char *buf) {

//     bool    request_good = parse_header_for_syntax();
//     if (!request_good)
//         return create_error(400);

//     _request._body_size = _request.isThereBody();
//     if (_request._body_size < 0) {
//         printErr(ERR9 "bad body-size");
//         return create_error(400);
//     }

//     _request._body = std::string(buf);
//     _request._body_bytes_received = _request._body.size();
//     // _buffer = "";
//     _buffer.clear();
    
//     _body_task = createTask(_request._method, _request, _answer);
//     if (_body_task->_status) {
//         std::cerr << "BodyTask status: " << _body_task->_status << std::endl;
//         // return create_error(_body_task->_status);
//     }
//     if (!_request._body_size || _request._body_bytes_received >= static_cast<size_t>(_request._body_size))
//         return DOING;
//     else 
//         return READING_BODY;
// }



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

