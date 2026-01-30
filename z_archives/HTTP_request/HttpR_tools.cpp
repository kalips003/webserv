#include "HttpR.hpp"
#include "Log.hpp"
#include "_colors.h"

#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <algorithm>

#include "Tools1.hpp"
#include "Tools2.hpp"
#include "SettingsServer.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** Wrapper for all the parsing and checking
* @return READING_BODY if body, DOING if not, errCode if error 	---*/
int	httpr::parsingHeaders(std::string& delim) {

// Move the start of the body into _body
	size_t pos = _buffer.find(delim);
	_body = _buffer.substr(pos + delim.size());
	_buffer = _buffer.substr(0, pos + 2); // leave one '\r\n' after the last header
	_body_bytes_received += _body.size();

	int errRtrn = parse_header_for_syntax();
	if (errRtrn != READING_BODY) {
		LOG_WARNING("SYNTAX ERROR - while parsing headers for syntax");
		return errRtrn;
	}

	errRtrn = parse_headers_for_validity();
	if (errRtrn > 100) {
		LOG_WARNING("SYNTAX ERROR - while parsing headers for validity");
		return errRtrn;
	}

	return errRtrn;
}

///////////////////////////////////////////////////////////////////////////////]
/** parse the headers to find if there is a body, return its length
*
* @return length of "body-size"
*
* if no "body-size" is found, return 0,
* if "body-size" is incorrect, return -1			---*/
ssize_t      httpr::isThereBody() const {

	map_istr::const_iterator it = _headers.find("content-length");
	if (it == _headers.end())
		return 0;
	int r;
	if (!atoi_v2(it->second, r) || r < 0)
		return -1;
	return static_cast<ssize_t>(r);
}


//-----------------------------------------------------------------------------]
/** Check the syntax validity of the path, doesnt check if file exist	
* a valid path has to start with '/'
---*/
bool	httpr::isPathValid(std::string& path) {

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
int httpr::parse_header_for_syntax() {

	std::vector<std::string> v;
	v = splitOnDelimitor(_buffer, "\r\n");
	if (!v.size()) {
		_buffer.clear();
		printErr(ERR9 "emtpy vector (you should never see this)");
		return 400;
	}

	std::vector<std::string>::iterator it = v.begin();
	++it; // move past first line: "GET /index.html HTTP/1.1"

	while (it != v.end()) {

		size_t colon_pos = it->find(':');
		if (colon_pos == std::string::npos) {
			LOG_WARNING("Bad header: ");
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
/** Parse request headers and validate body-related information.
 *
 * - Determines whether the request has a body and its expected size.
 * - Initializes storage for the request body if present.
 * - Creates a temporary file to store the body data.
 *
 * @return
 *   - DOING if there is no body, or if the full body has already been received.
 *   - READING_BODY if the body exists but is not fully received yet.
 *   - An HTTP error code (400, 500) on failure.	---*/
int    httpr::parse_headers_for_validity() {

	_body_size = isThereBody();
	if (_body_size < 0) {
		LOG_ERROR("SYNTAX ERROR - Bad body-size: " << _headers.find("content-length")->second);
		return 400;
	}
	if (!_body_size)
		return DOING;

	std::string temp_file_name;
	int	fd = createTempFile(temp_file_name, g_settings.find_setting("tmp_root"), O_WRONLY | O_CREAT | O_EXCL);
	if (fd < 0)
		return 500;

	if (write(fd, _body.c_str(), _body.size()) < static_cast<ssize_t>(_body.size())) {
		printErr("write()");
		close(fd);
		return 500;
	}

	_fd_body = fd;
	_tmp_body_path = temp_file_name;

	if (_body_bytes_received >= static_cast<size_t>(_body_size))
		return DOING;
	else 
		return READING_BODY;
}

///////////////////////////////////////////////////////////////////////////////]
// 		GETTERS / SETTERS
///////////////////////////////////////////////////////////////////////////////]

//-----------------------------------------------------------------------------]
/** find the given setting in the _headers
*	@return the string value of the setting
*
* if setting not found, return "" empty string	---*/
std::string httpr::find_setting(const std::string& set) const {

	map_strstr::const_iterator it = _headers.begin();
	it = _headers.find(set);
	if (it == _headers.end()) {
		LOG_ERROR(RED "setting not found: " RESET);
		return "";
	}
	else 
		return it->second;
}

// #include <sys/stat.h>
// #include <fcntl.h>
//-----------------------------------------------------------------------------]
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
int	httpr::openFdBody(const char* path) {

	_fd_body = open(path, O_CREAT | O_RDWR | O_TRUNC, 0600);

	if (_fd_body < 0)
		return printErr("open()"), 400;
 
	_tmp_body_path = path;

	write(_fd_body, _body.c_str(), _body.size());

	_body.clear();

	return READING_BODY;
}