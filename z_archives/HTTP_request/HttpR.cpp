#include "HttpR.hpp"
#include "Log.hpp"
#include "_colors.h"

#include <unistd.h>

#include <iostream>

#include "Tools1.hpp"
#include "HttpMethods.hpp"

///////////////////////////////////////////////////////////////////////////////]
/**	Destroy the associated _fd_body, annd unlink the _tmp_file */
httpr::~httpr() {
	if (!_tmp_body_path.empty()) unlink(_tmp_body_path.c_str());
	if (_fd_body >= 0) close(_fd_body);
}
///////////////////////////////////////////////////////////////////////////////]
; // 411 if "content lenght missing"

// #include "HttpMethods.hpp"
// #include <sstream>
///////////////////////////////////////////////////////////////////////////////]
/** Add given string to _buffer, parse it for "\r\n"
* 
* if found, parse the HEAD of request, and remove it from _buffer
* 
* @return if delim found, return READING_HEADER if HEAD valid, errCode if not
* 
* if delim ("\r\n") not found, return FIRST			---*/
int    httpr::readingFirstLine(std::string& str_buff) {

	std::string temp = _buffer + str_buff;

	size_t  pos = temp.find("\r\n");
	if (pos == std::string::npos) {
		_buffer += str_buff;

		if (_buffer.size() > MAX_LIMIT_FOR_HEAD) {
			LOG_WARNING("Max Limit (" RED << MAX_LIMIT_FOR_HEAD << RESET ") reached before finding CRLF");
			return 400;
		}
		return FIRST;
	}

	std::stringstream ss(temp.substr(0, pos)); // "GET /index.html HTTP/1.1"
	std::string word;

	if (!(ss >> word) || isMethodValid(word) < 0) {
		LOG_WARNING(RED "Invalid Method: " RESET << word);
		return 501; // or 400
	}
	_method = word;

	if (!(ss >> word) || !isPathValid(word)) {
		LOG_WARNING(RED "Invalid Path: " RESET << word);
		return 400;
	}
	_path = word;

	if (!(ss >> word) || word != "HTTP/1.1") {
		LOG_WARNING(RED "Invalid Version: " RESET << word);
		return 505;
	}
	_version = word;

	if (ss >> word) { // extra garbage after the 3 tokens
		LOG_WARNING(RED "Invalid HEAD: " RESET << _buffer.substr(0, _buffer.find("\r\n")));
		return 400;
	}

	return READING_HEADER;
}

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
int    httpr::readingHeaders(std::string& buff) {

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

///////////////////////////////////////////////////////////////////////////////]
/**	Manage the new read and append it to the correct body 
*
* if body small, append to _body
* if big, write to _fd_body
*
* @return DOING if body finished to be received, READING_BODY otherwise	---*/
int	httpr::readingBody(std::string& buff) {

	if (_fd_body >= 0) {
	
		if (write(_fd_body, buff.c_str(), buff.size()) < static_cast<ssize_t>(buff.size())) {
			LOG_ERROR("write()");
			return 500;
		}
	}

	_body_bytes_received += buff.size();

	if (_body_bytes_received >= static_cast<size_t>(_body_size))
		return DOING;
	return READING_BODY;
}


///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, httpr& r) {

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

