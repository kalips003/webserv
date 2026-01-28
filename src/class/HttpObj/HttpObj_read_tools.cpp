#include "HttpObj.hpp"
#include "Log.hpp"

#include <algorithm>

#include "Tools1.hpp"

#include "SettingsServer.hpp"

///////////////////////////////////////////////////////////////////////////////]
/***  					READING	PARSING HEADERS								***/
///////////////////////////////////////////////////////////////////////////////]
/** @brief Parses headers and initializes body handling.
 *
 * Called once the end of Headers has been found, and isolated in _buffer, 
 * (_leftovers still holds the body if any)
 *
 * Validates syntax, determines expected body size, and enforces
 * 
 * If a body is expected, creates a temp file and writes any leftover body
 * bytes into it.
 *
 * @return READING_STATUS or errCode on error	---*/
int	HttpObj::parseHeaders() {
LOG_DEBUG("entering parseHeaders()");

	int errRtrn = parse_buffer_for_headers();
	if (errRtrn)
		return errRtrn;
LOG_DEBUG("return of: parse_buffer_for_headers(): " << errRtrn);

	_bytes_total = isThereBodyinHeaders();
	if (_bytes_total < 0) {
		LOG_ERROR( "SYNTAX ERROR - Bad body-size: " << _headers.find("content-length")->second);
		return 400;
	}
	else if (!_bytes_total) {
		int rtrn = this->validateBodyWithoutLength(); // if "content-length" from POST > 411 Length Required
		LOG_DEBUG("return of: validateBodyWithoutLength(): " << rtrn);
		_leftovers.clear();
		return rtrn; // normally return DOING;
	}
	else if (_bytes_total > g_settings.getMaxBodySize())
		return 413; // 413 Payload Too Large

	if (!_tmp_file.createTempFile(&g_settings.getTempRoot()))
		return 500;
	
	ssize_t wr_rtrn = write(_tmp_file._fd, _leftovers.c_str(), _leftovers.size());
	if (wr_rtrn < static_cast<ssize_t>(_leftovers.size())) {
		LOG_ERROR_SYS("parseHeaders(): write()");
		return 500;
	}
	_bytes_written = static_cast<size_t>(wr_rtrn);
	_leftovers.clear();

	if (_bytes_written >= static_cast<size_t>(_bytes_total))
		return DOING;
	else 
		return READING_BODY;
}


// #include <algorithm>
//-----------------------------------------------------------------------------]
/** @brief Parses _buffer containing CRLF-delimited HTTP headers into _headers.
 *
 * - Splits _buffer on "\r\n".
 * - Each line must contain a colon ':' separating key and value.
 * - Trims whitespace; keys are lowercased; empty values are allowed.
 * - Does not validate header semantics, only syntax.
 * - Clears _buffer after parsing.
 *
 * @return 0 on success, errCode (e.g., 400) on error. ---*/
int HttpObj::parse_buffer_for_headers() {

	std::vector<std::string> v;
	v = splitOnDelimitor(_buffer, "\r\n");

	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {

		size_t colon_pos = it->find(':');
		if (colon_pos == std::string::npos) {
			oss msg; msg << C_512 "Bad header: {" RESET << *it << C_512 "}" RESET; printLog(ERROR, msg.str(), 1);
			_buffer.clear();
			return 400;
		}

		std::string key = trim_white(it->substr(0, colon_pos));
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		std::string value = trim_white(it->substr(colon_pos + 1));
		_headers[key] = value;
	}

	_buffer.clear();
	return 0;
}

