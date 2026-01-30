#include "HttpRequest.hpp"
#include "Log.hpp"

#include "_colors.h"

#include <iostream>
#include <algorithm>

#include "Tools1.hpp"
#include "HttpMethods.hpp"

///////////////////////////////////////////////////////////////////////////////]
; // 411 if "content lenght missing"



// #include "HttpMethods.hpp"
//-----------------------------------------------------------------------------]
/** Parse the content of _buffer into > _method, _path, _version
// @return 0 for success, errCode else	---*/
int HttpRequest::isFirstLineValid(int fd) {

	std::stringstream ss(_buffer); // "GET /index.html HTTP/1.1"
	std::string word;

	if (!(ss >> word) || isMethodValid(word) < 0) {
		LOG_ERROR(RED "Invalid Method: " RESET << word);
		return 501; // or 400
	}
	_method = word;
	
	if (!(ss >> word) || word[0] != '/') {
		LOG_ERROR(RED "Invalid Path (not absolute): " RESET << word);
		return 400;
	}
	_path = word;
	LOG_LOG("isFirstLineValid(): _path: " << _path);

	if (!(ss >> word) || word != "HTTP/1.1") { // ??? - Handle other versions?
		LOG_ERROR(RED "Invalid Version: " RESET << word);
		return 505;
	}
	_version = word;

	if (ss >> word) { // extra garbage after the 3 tokens
		LOG_ERROR(RED "Invalid HEAD: " RESET << _buffer);
		return 400;
	}

	LOG_LOG(printFd(fd) << "→ " << _method << " " << _path);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////]
int		HttpRequest::validateBodyWithoutLength() {
	if ((_method == "POST" || _method == "PUT") && !find_setting("content-length"))
		return 411; // 411 Length Required
	return static_cast<int>(DOING);
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const HttpRequest& r) {

	os << C_542 "-------------HTTP REQUEST--------------------\n" RESET;
	os << C_542 "\tREQUEST:\n" RESET;
	// os << C_410 << r._method << " " C_144 << r._path << " " C_232 << r._version << RESET << std::endl;

	os << C_542 "\tHTTP Object:\n" RESET << static_cast<const HttpObj&>(r);

	os << C_542 "_body_size_to_expect: " RESET << r._bytes_total << std::endl;
	os << C_542 "_body_bytes_received: " RESET << r._bytes_written << std::endl;
	return os;
}

