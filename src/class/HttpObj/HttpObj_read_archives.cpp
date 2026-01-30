#include "HttpObj.hpp"

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from fd into _buffer.
// Buffer is then written into _tmp_file
//
* @return READING_STATUS or errCode on error		---*/
// int    HttpObj::receive(char *buff, size_t sizeofbuff, int fd) {

// 	int rtrn;
// 	if (_status == READING_FIRST) {
// 		LOG_DEBUG("receive(): READING_FIRST")
// 		rtrn = readingFirstLine(buff, sizeofbuff, fd);
// 		if (rtrn >= 100)
// 			return rtrn;
// 		_status = static_cast<HttpBodyStatus>(rtrn);
// 	}

// 	if (_status == READING_HEADER) {
// 		LOG_DEBUG("receive(): READING_HEADER")
// 		rtrn = readingHeaders(buff, sizeofbuff, fd);
// 		LOG_LOG("receive(): AFTER READING_HEADER: " << rtrn)
// 		if (rtrn >= 100)
// 			return rtrn;
// 		_status = static_cast<HttpBodyStatus>(rtrn);
// 	}

// 	if (_status == READING_BODY) {
// 		LOG_DEBUG("receive(): READING_BODY")
// 		rtrn = readingBody(buff, sizeofbuff, fd);
// 		if (rtrn >= 100)
// 			return rtrn;
// 		_status = static_cast<HttpBodyStatus>(rtrn);
// 	}

// 	if (_status == CLOSED) {
// 		LOG_DEBUG("receive(): CLOSED")
// 		oss msg; msg << "[#" << printFd(fd) << "] → " RED "Connection closed (FIN received)" RESET; printLog(INFO, msg.str(), 1);
// 	}
// 	return _status;
// }

// #include "HttpMethods.hpp"
// #include <sstream>
///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from fd into _buffer until "\r\n" is found.
//
// - If found: validates it, clears _buffer,
// leaves _leftovers, returns READING_HEADER.
// 
// - If not found: updates _bytes_received, returns READING_FIRST,
// 
// @param buff       Temporary read buffer
// @param sizeofbuff Buffer size
// @param fd         File descriptor
// @return READING_STATUS or errCode on error (or MAX_LIMIT_FOR_HEAD reached)	---*/
// int    HttpObj::readingFirstLine(char *buff, size_t sizeofbuff, int fd) {

// 	bool	is_found = false;
// 	ssize_t bytes_read = read_until_delim_is_found(buff, sizeofbuff, fd, "\r\n", is_found);

// 	if (bytes_read == 0)
// 		return CLOSED;
// 	else if (bytes_read < 0)
// 		return READING_FIRST;
// 	if (is_found) {
// 		int rtrn = this->isFirstLineValid(fd);
// 		if (rtrn)
// 			return rtrn;

// 		LOG_DEBUG("readingFirstLine() is found: " << _buffer);
// 		_buffer.clear(); // empty _buffer, leave _leftovers untouched, next function will decide what to do with it
// 		_bytes_written = 0;
// 		return READING_HEADER;
// 	}
// 	else {
// 		_bytes_written += bytes_read;
// 		if (_bytes_written > MAX_LIMIT_FOR_HEAD) {
// 			oss msg; msg << "Max Limit (" RED << MAX_LIMIT_FOR_HEAD << RESET ") reached before finding CRLF"; printLog(ERROR, msg.str(), 1);
// 			return 400;
// 		}
// 	}
// 	return READING_FIRST;
// }


///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from fd into _buffer until '\r\n\r\n' is found.
//
// - If found: parse headers, handle the creating of the temp file if an body (and _leftovers)
// 
// - If not found: updates _bytes_received, returns READING_HEADER,
// 
* @return READING_STATUS or errCode on error	---*/
// int		HttpObj::readingHeaders(char *buff, size_t sizeofbuff, int fd) {

// 	std::string	delim = "\r\n\r\n";
// 	bool		is_found = false;

// 	LOG_DEBUG("_buffer: {" << _buffer << "} _leftover: {" << _leftovers << "}");
// 	ssize_t bytes_read = 1; // hack
// 	if (_leftovers.empty()) // nothing left from the parsing of the first line
// 		bytes_read = read_until_delim_is_found(buff, sizeofbuff, fd, delim, is_found);
// 	else {
// 		size_t pos = _leftovers.find(delim);
// 		if (pos != std::string::npos) { // if all the headers are in the _leftovers
// 			_buffer = _leftovers.substr(0, pos);
// 			_leftovers.erase(0, pos + delim.size());
// 			is_found = true;
// 			LOG_DEBUG("AFTER _buffer: {" << _buffer << "} _leftover: {" << _leftovers << "}");
// 		}
// 		else { // if there are still headers to read
// 			_buffer = _leftovers;
// 			_leftovers.clear();
// 			_bytes_written = _buffer.size();
// 			return READING_HEADER;
// 		}
// 	}

// 	if (bytes_read == 0)
// 		return CLOSED;
// 	else if (bytes_read < 0)
// 		return READING_HEADER;

// 	if (is_found)
// 		return parseHeaders();
// 	else {
// 		_bytes_written += bytes_read;
// 		if (_bytes_written > MAX_LIMIT_FOR_HEADERS) {
// 			oss msg; msg << "Max Limit (" RED << MAX_LIMIT_FOR_HEADERS << RESET ") reached before finding '\\r\\n\\r\\n'"; printLog(ERROR, msg.str(), 1);
// 			return 400;
// 		}
// 	}
// 	return READING_HEADER;
// }


///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from fd into _buffer.
// Buffer is then written into _tmp_file
//
* @return READING_STATUS or errCode on error		---*/
// int    HttpObj::readingBody(char *buff, size_t sizeofbuff, int fd) {

	
// 	ssize_t	bytes_received = readBuffer(buff, sizeofbuff, fd, _buffer);

// 	if (bytes_received < 0)
// 		return READING_BODY;
// 	else if (!bytes_received)
// 		return CLOSED;

// 	ssize_t rtrn_write;
// 	ssize_t bytes_written = 0;
// 	while (bytes_written < bytes_received) {
	
// 		if ((rtrn_write = write(_tmp_file._fd, _buffer.c_str() + bytes_written, bytes_received - bytes_written)) < 0) {
// 			printErr("readingBody(): write()");
// 			return 500;
// 		}
// 		bytes_written += rtrn_write;
// 	}
// 	_bytes_written += bytes_written;
// 	_buffer.clear();

// 	if (_bytes_written >= static_cast<size_t>(_bytes_total))
// 		return DOING;
// 	return READING_BODY;
// }

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
// int	HttpObj::parseHeaders() {
// LOG_DEBUG("entering parseHeaders()");

// 	int errRtrn = parse_buffer_for_headers();
// 	if (errRtrn)
// 		return errRtrn;
// 	LOG_DEBUG("return of: parse_buffer_for_headers(): " << errRtrn);

// 	_bytes_total = isThereBodyinHeaders();
// 	if (_bytes_total < 0) {
// 		LOG_ERROR( "SYNTAX ERROR - Bad body-size: " << _headers.find("content-length")->second);
// 		return 400;
// 	}
// 	else if (!_bytes_total) {
// 		int rtrn = this->validateBodyWithoutLength(); // if "content-length" from POST > 411 Length Required
// 		LOG_DEBUG("return of: validateBodyWithoutLength(): " << rtrn);
// 		_leftovers.clear();
// 		return rtrn; // normally return DOING;
// 	}
// 	else if (g_settings.getMaxBodySize() != -1 && (_bytes_total > g_settings.getMaxBodySize())) // < ???
// 	// else if (_bytes_total > g_settings.getMaxBodySize())
// 		return 413; // 413 Payload Too Large

// 	if (!_tmp_file.createTempFile(&g_settings.getTempRoot()))
// 		return 500;
	
// 	ssize_t wr_rtrn = write(_tmp_file._fd, _leftovers.c_str(), _leftovers.size());
// 	if (wr_rtrn < static_cast<ssize_t>(_leftovers.size())) {
// 		LOG_ERROR_SYS("parseHeaders(): write()");
// 		return 500;
// 	}
// 	_bytes_written = static_cast<size_t>(wr_rtrn);
// 	_leftovers.clear();

// 	if (_bytes_written >= static_cast<size_t>(_bytes_total))
// 		return DOING;
// 	else 
// 		return READING_BODY;
// }

//-----------------------------------------------------------------------------]
/** @brief Reads from a file descriptor into _buffer until a specified delimiter is found.
 *
 * Appends new data from the FD to _buffer, checking for the delimiter.
 * If the delimiter is found:
 *   - _buffer is truncated to just before the delimiter
 *   - _leftovers stores any remaining data after the delimiter (included)
 *   - is_found is set to true
 *
 * @param buff       Temporary buffer for reading chunks
 * @param sizeofbuff Size of the temporary buffer
 * @param fd         File descriptor to read from (socket or file)
 * @param delimitor  Delimiter string to search for
 * @param is_found   Output flag set to true when delimiter is found
 *
 * @return Number of bytes read in this call, or <= 0 on EOF or error	---*/
// ssize_t HttpObj::read_until_delim_is_found(char *buff, size_t sizeofbuff, int fd, const std::string& delimitor, bool& is_found) {

// // copy the last bytes of buff_to_append
// 	size_t n = _buffer.size() > delimitor.size() ? delimitor.size() - 1 : _buffer.size();

// // append the return of recv()
// 	ssize_t bytes_recv = readBuffer(buff, sizeofbuff, fd, _buffer);
// 	if (bytes_recv <= 0)
// 		return bytes_recv;

// 	size_t pos = _buffer.find(delimitor, n);
// 	if (pos == std::string::npos)
// 		return bytes_recv;

// // if found, does the splitting
// 	_leftovers = _buffer.substr(pos);
// 	_buffer.resize(pos);
// 	is_found = true;
	
// 	return bytes_recv;
// }




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
// int HttpObj::parse_buffer_for_headers() {

// 	std::vector<std::string> v;
// 	v = splitOnDelimitor(_buffer, "\r\n");

// 	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {

// 		size_t colon_pos = it->find(':');
// 		if (colon_pos == std::string::npos) {
// 			oss msg; msg << C_512 "Bad header: {" RESET << *it << C_512 "}" RESET; printLog(ERROR, msg.str(), 1);
// 			_buffer.clear();
// 			return 400;
// 		}

// 		std::string key = trim_white(it->substr(0, colon_pos));
// 		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
// 		std::string value = trim_white(it->substr(colon_pos + 1));
// 		_headers[key] = value;
// 	}

// 	_buffer.clear();
// 	return 0;
// }