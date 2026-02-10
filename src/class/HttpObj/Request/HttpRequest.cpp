#include "HttpRequest.hpp"
#include "Log.hpp"

#include "_colors.h"

#include <iostream>
#include <algorithm>

#include "Tools1.hpp"
#include "HttpMethods.hpp"
#include "Settings.hpp"

///////////////////////////////////////////////////////////////////////////////]
// #include "HttpMethods.hpp"
//-----------------------------------------------------------------------------]
/** Parse the content of _first into > _method, _path, _version
// @return 0 for success, errCode else	---*/
int HttpRequest::isFirstLineValid(int fd) {

	std::stringstream ss(_first); // "GET /index.html HTTP/1.1"
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
	LOG_DEBUG("isFirstLineValid(): _path: " << _path);

	if (!(ss >> word) || word != "HTTP/1.1") { // ??? - Handle other versions?
		LOG_ERROR(RED "Invalid Version: " RESET << word);
		return 505;
	}
	_version = word;

	if (ss >> word) { // extra garbage after the 3 tokens
		LOG_ERROR(RED "Invalid HEAD: " RESET << _first);
		return 400;
	}

	LOG_LOG(printFd(fd) << "→ " << _method << " " << _path);
	return 0;
}

// #include "Settings.hpp"
///////////////////////////////////////////////////////////////////////////////]
int		HttpRequest::parseHeadersForValidity() {

	_bytes_total = isThereBodyinHeaders();
	if (_bytes_total < 0) {
		LOG_ERROR( "SYNTAX ERROR - Bad body-size: " << _headers.find("content-length")->second);
		return 400;
	}
	int rtrn = validateLocationBlock(_bytes_total);
	if (rtrn)
		return rtrn;
	if (!_bytes_total)
		return detectChunkedEncoding();

	if (_bytes_total != 0 && !(_method == "POST" || _method == "PUT" || _method == "PATCH"))
		return HttpObj::DOING;

// there is a body, we create it
	if (!_tmp_file.createTempFile(&g_settings.getTempRoot()))
		return 500;
	
	size_t to_copy = static_cast<size_t>(_bytes_total) <= _leftovers.size() ? _bytes_total : _leftovers.size();
	ssize_t wr_rtrn = write(_tmp_file._fd, _leftovers.c_str(), to_copy);
	if (wr_rtrn < static_cast<ssize_t>(to_copy)) {
		LOG_ERROR_SYS("HttpRequest::parseHeadersForValidity(): write()");
		return 500;
	}
	_bytes_written = static_cast<size_t>(wr_rtrn);
	_leftovers.clear();

	if (_bytes_written >= static_cast<size_t>(_bytes_total))
		return HttpObj::DOING;
	else 
		return HttpObj::READING_BODY;
}

/** parse given path to extract exact path, find correct block, 
// validate if method allowed, and body size ok */
int		HttpRequest::validateLocationBlock(ssize_t body_size) {

	std::string path;

	size_t pos = _path.find_first_of('?');
	if (pos == std::string::npos)
		path = _path;
	else 
		path = _path.substr(0, pos);
	
// sanitize given_path for %XX;
	std::string sanitized;
	if (Settings::sanitizePath(sanitized, path))
		return 400;
	LOG_DEBUG("validateLocationBlock(): path after sanitizePath: " << sanitized);
// find location block from all the location /blocks
	const Settings::block* location;
	if (!(location = Settings::isLocationKnown(sanitized, *_settings))) {
		LOG_ERROR("CAN'T FIND LOCATION BLOCK: " << _path)
		return 500; // could also be 404;
	}
	if (Settings::getFullPath(sanitized, sanitized, *_settings)) // check for escaping root
		return 403;
// Once Location block is known, check if method is allowed
	std::string method_to_check = _method;
	if (_method == "HEAD") method_to_check = "GET";
	if (std::find(location->data.allowed_methods.begin(), location->data.allowed_methods.end(), method_to_check) == location->data.allowed_methods.end()) {
		LOG_WARNING("Method " << _method << " not allowed in: " << location->path);
		return 405;
	}
// check is method require a body
	if ((_method == "POST" || _method == "PUT" || _method == "PATCH") && !body_size && !find_in_headers("Transfer-Encoding")) {
		LOG_WARNING("Method " << _method << " require a body");
		return 411; // 411 Length Required
	}
// 	validate body_size based on block allowed max size
	if (location->data.client_max_body_size != -1 && (body_size > location->data.client_max_body_size)) {
		LOG_WARNING(location->path << ": Max body size " << location->data.client_max_body_size << " < Requested POST body: " << body_size);
		return 413; // 413 Payload Too Large
	}
	return 0;
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

