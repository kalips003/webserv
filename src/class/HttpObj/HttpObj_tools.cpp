#include "HttpObj.hpp"

#include <string.h>
#include <ctime>
#include <string>
#include <algorithm>

#include "Log.hpp"
#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
/***  								TOOLS									***/
///////////////////////////////////////////////////////////////////////////////]
//-----------------------------------------------------------------------------]
/**	Return what to send next (SENDING_HEAD | SENDING_BODY | SENDING_BODY_FD | CLOSED)	---*/
HttpObj::HttpBodyStatus	HttpObj::whatToSend() const {

	if (!_head.empty())
		return SENDING_HEAD;
	if (!_body.empty())
		return SENDING_BODY;
	if (_tmp_file._fd >= 0)
		return SENDING_BODY_FD;
	return CLOSED;
}

//-----------------------------------------------------------------------------]
/** @brief Concatenates _first line and _headers into _head with CRLFs. */
void	HttpObj::concatenateIntoHead() {
	_head.reserve(4096);
	_head = _first;
	if (_first.substr(_first.size() - 2) != "\r\n")
		_head += "\r\n";

	for (map_strstr::iterator it = _headers.begin(); it != _headers.end(); it++)
		_head += it->first + ": " + it->second + "\r\n";
	_head += "\r\n";
}

// #include <algorithm>
//-----------------------------------------------------------------------------]
/** @brief Parses _head containing CRLF-delimited HTTP headers into _headers.
 *
 * - Splits _buffer on "\r\n".
 * - Each line must contain a colon ':' separating key and value.
 * - Trims whitespace; keys are lowercased; empty values are allowed.
 * - Does not validate header semantics, only syntax.
 * - Clears _head after parsing.
 *
 * @return 0 on success, errCode (e.g., 400) on error. ---*/
int HttpObj::parse_head_for_headers() {

	std::vector<std::string> v;
	v = splitOnDelimitor(_head, "\r\n");

	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {

		size_t colon_pos = it->find(':');
		if (colon_pos == std::string::npos) {
			LOG_ERROR(C_512 "Bad header: {" RESET << *it << C_512 "}" RESET);
			return 400;
		}

		std::string key = trim_white(it->substr(0, colon_pos));
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		std::string value = trim_white(it->substr(colon_pos + 1));
		_headers[key] = value;
	}

	_head.clear();
	return 0;
}


///////////////////////////////////////////////////////////////////////////////]
/***  								GETTERS									***/
///////////////////////////////////////////////////////////////////////////////]
#include <algorithm>
#include <ctype.h>
//-----------------------------------------------------------------------------]
/** find the given setting in the _headers
*	@return the string value of the setting
*
* if setting not found, return NULL	---*/
const std::string*	HttpObj::find_in_headers(const std::string& set) const {

	std::string to_find = set;
	std::transform(to_find.begin(), to_find.end(), to_find.begin(), ::tolower);

	map_strstr::const_iterator it = _headers.begin();

	it = _headers.find(to_find);
	if (it == _headers.end()) {
		LOG_DEBUG(C_024 "HttpObj::find_setting() not found: " RESET << set);
		return NULL;
	}
	else 
		return &it->second;
}
//-----------------------------------------------------------------------------]
std::string*	HttpObj::find_in_headers(const std::string& set) {

	std::string to_find = set;
	std::transform(to_find.begin(), to_find.end(), to_find.begin(), ::tolower);

	map_strstr::iterator it = _headers.begin();

	it = _headers.find(to_find);
	if (it == _headers.end()) {
		LOG_DEBUG(C_024 "HttpObj::find_setting() not found: " RESET << set);
		return NULL;
	}
	else 
		return &it->second;
}

//-----------------------------------------------------------------------------]
/** parse the headers to find if there is a body, return its length
*
* @return length of "content-length"
*
* if no "content-length" is found, return 0,
* if "content-length" is incorrect, return -1			---*/
ssize_t      HttpObj::isThereBodyinHeaders() const {

	const std::string* size = find_in_headers("Content-Length");
	if (!size)
		return 0;
	int r;
	if (!atoi_v2(*size, r) || r < 0)
		return -1;
	return static_cast<ssize_t>(r);
}

//-----------------------------------------------------------------------------]
/** @return size of _body or size of _temp_file */
size_t	HttpObj::isThereBody() {

	if (!_body.empty()) {
		if (_tmp_file._fd >= 0) LOG_ERROR(RED "YOU SHOULD NEVER SEE THIS: " RESET "_body exist and fd also open");
		return _body.size();
	}
	if (_tmp_file._fd >= 0)
		return _tmp_file.getBodySize();

	return 0;
}


///////////////////////////////////////////////////////////////////////////////]
/***  								SETTERS									***/
///////////////////////////////////////////////////////////////////////////////]
// #include <ctime>
// #include <string>
// #include <algorithm>
//-----------------------------------------------------------------------------]
/** Set default headers for an Answer		---*/
void	HttpObj::setDefaultHeaders() {
	_headers["server"] = "Webserv/0.1";

	std::string* set = find_in_headers("Content-Type");
	if (set)
		std::replace(set->begin(), set->end(), '|', ';');
	else
		_headers["content-type"] = "text/html| charset=utf-8";
	
	if (!find_in_headers("Connection"))
		_headers["connection"] = "close";
	if (!find_in_headers("Cache-Control"))
		_headers["cache-control"] = "no-cache"; // no-cache or max-age=3600

// DATE
	// get current time in UTC
	std::time_t t = std::time(NULL);
	std::tm tm_utc;
	gmtime_r(&t, &tm_utc); // thread-safe, fills tm_utc with UTC time

	char buf[30]; // enough for "Day, DD Mon YYYY HH:MM:SS GMT"
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_utc);
	_headers["date"] = std::string(buf);

// Last modified timestamp of the file
	if (_tmp_file._fd != -1) {
		_tmp_file.updateStat();
		t = _tmp_file._info.st_mtime;
		gmtime_r(&t, &tm_utc);
		std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_utc);
		_headers["last-modified"] = std::string(buf);
	}
}

#include "Tools2.hpp"
//-----------------------------------------------------------------------------]
void	HttpObj::setMIMEtype(const std::string& path) {
	_headers["content-type"] = find_MIME_type(path);
}

//-----------------------------------------------------------------------------]
void	HttpObj::closeTemp(bool del) {
	_tmp_file.closeTemp(del);
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const HttpObj& r) {

	os << C_542 "-----------HTTP OBJECT---------------------\n" RESET;
	os << C_542 "\t_first: {\n" RESET << r._first << C_542 "}" RESET << std::endl;
	os << C_542 "\t_head: {\n" RESET << r._head << C_542 "}" RESET << std::endl;

	os << C_542 "\t_headers:{\n" RESET;
	for (map_strstr::const_iterator  h = r._headers.begin(); h != r._headers.end(); h++)
		os << C_114 << h->first << RESET ": " << h->second << std::endl;
	os << C_542 "}" RESET << std::endl;

	os << C_542 "\t_body: {\n" RESET << r._body << C_542 "}" RESET << std::endl;
	os << C_542 "\t_buffer: {\n" RESET << r._buffer << C_542 "}" RESET << std::endl;
	os << C_542 "\t_leftovers: {\n" RESET << r._leftovers << C_542 "}" RESET << std::endl;

	os << C_542 "\t_tmp_file: {\n" RESET << r._tmp_file << C_542 "}" RESET << std::endl;
	
	return os;
}


