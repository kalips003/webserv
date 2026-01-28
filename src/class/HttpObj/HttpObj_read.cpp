#include "HttpObj.hpp"

#include "Tools1.hpp"
#include "Log.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from fd into _buffer.
// Buffer is then written into _tmp_file
//
* @return READING_STATUS or errCode on error		---*/
int    HttpObj::receive(char *buff, size_t sizeofbuff, int fd) {

	int rtrn;
	if (_status == READING_FIRST) {
		LOG_DEBUG("receive(): READING_FIRST")
		rtrn = readingFirstLine(buff, sizeofbuff, fd);
		if (rtrn >= 100)
			return rtrn;
		_status = static_cast<HttpBodyStatus>(rtrn);
	}

	if (_status == READING_HEADER) {
		LOG_DEBUG("receive(): READING_HEADER")
		rtrn = readingHeaders(buff, sizeofbuff, fd);
		LOG_LOG("receive(): AFTER READING_HEADER: " << rtrn)
		if (rtrn >= 100)
			return rtrn;
		_status = static_cast<HttpBodyStatus>(rtrn);
	}

	if (_status == READING_BODY) {
		LOG_DEBUG("receive(): READING_BODY")
		rtrn = readingBody(buff, sizeofbuff, fd);
		if (rtrn >= 100)
			return rtrn;
		_status = static_cast<HttpBodyStatus>(rtrn);
	}

	if (_status == CLOSED) {
		LOG_DEBUG("receive(): CLOSED")
		oss msg; msg << "[#" << printFd(fd) << "] → " RED "Connection closed (FIN received)" RESET; printLog(INFO, msg.str(), 1);
	}
	return _status;
}

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
int    HttpObj::readingFirstLine(char *buff, size_t sizeofbuff, int fd) {

	bool	is_found = false;
	ssize_t bytes_read = read_until_delim_is_found(buff, sizeofbuff, fd, "\r\n", is_found);

	if (bytes_read == 0)
		return CLOSED;
	else if (bytes_read < 0)
		return READING_FIRST;
	if (is_found) {
		int rtrn = this->isFirstLineValid(fd);
		if (rtrn)
			return rtrn;

		LOG_DEBUG("readingFirstLine() is found: " << _buffer);
		_buffer.clear(); // empty _buffer, leave _leftovers untouched, next function will decide what to do with it
		_bytes_written = 0;
		return READING_HEADER;
	}
	else {
		_bytes_written += bytes_read;
		if (_bytes_written > MAX_LIMIT_FOR_HEAD) {
			oss msg; msg << "Max Limit (" RED << MAX_LIMIT_FOR_HEAD << RESET ") reached before finding CRLF"; printLog(ERROR, msg.str(), 1);
			return 400;
		}
	}
	return READING_FIRST;
}


///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from fd into _buffer until "\r\n\r\n" is found.
//
// - If found: parse headers, handle the creating of the temp file if an body (and _leftovers)
// 
// - If not found: updates _bytes_received, returns READING_HEADER,
// 
* @return READING_STATUS or errCode on error	---*/
int		HttpObj::readingHeaders(char *buff, size_t sizeofbuff, int fd) {

	std::string delim = "\r\n\r\n";
	bool	is_found = false;

	LOG_DEBUG("_buffer: {" << _buffer << "} _leftover: {" << _leftovers << "}");
	ssize_t bytes_read = 1; // hack
	if (!_leftovers.size()) // nothing left from the parsing of the first line
		bytes_read = read_until_delim_is_found(buff, sizeofbuff, fd, delim, is_found);
	else {
		size_t pos = _leftovers.find(delim);
		if (pos != std::string::npos) { // if all the headers are in the _leftovers
			_buffer = _leftovers.substr(0, pos);
			_leftovers.resize(pos + delim.size());
			is_found = true;
		}
		else { // if there are still headers to read
			_buffer = _leftovers;
			_leftovers.clear();
			_bytes_written = _buffer.size();
			return READING_HEADER;
		}
	}

	if (bytes_read == 0)
		return CLOSED;
	else if (bytes_read < 0)
		return READING_HEADER;

	if (is_found)
		return parseHeaders();
	else {
		_bytes_written += bytes_read;
		if (_bytes_written > MAX_LIMIT_FOR_HEADERS) {
			oss msg; msg << "Max Limit (" RED << MAX_LIMIT_FOR_HEADERS << RESET ") reached before finding '\\r\\n\\r\\n'"; printLog(ERROR, msg.str(), 1);
			return 400;
		}
	}
	return READING_HEADER;
}


///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from fd into _buffer.
// Buffer is then written into _tmp_file
//
* @return READING_STATUS or errCode on error		---*/
int    HttpObj::readingBody(char *buff, size_t sizeofbuff, int fd) {

	
	ssize_t	bytes_received = readBuffer(buff, sizeofbuff, fd, _buffer);

	if (bytes_received < 0)
		return READING_BODY;
	else if (!bytes_received)
		return CLOSED;

	ssize_t rtrn_write;
	ssize_t bytes_written = 0;
	while (bytes_written < bytes_received) {
	
		if ((rtrn_write = write(_tmp_file._fd, _buffer.c_str() + bytes_written, bytes_received - bytes_written)) < 0) {
			printErr("readingBody(): write()");
			return 500;
		}
		bytes_written += rtrn_write;
	}
	_bytes_written += bytes_written;
	_buffer.clear();

	if (_bytes_written >= static_cast<size_t>(_bytes_total))
		return DOING;
	return READING_BODY;
}

