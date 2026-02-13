#include "HttpObj.hpp"

#include "Tools1.hpp"
#include "Server.hpp"

#include <string.h>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////]
/***  								READ									***/
///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from a file descriptor and leftover buffer to fill the HttpObj.
 *
 * Updates internal state as it parses the HTTP request in stages:
 * 1. READING_FIRST
 *    - Consume _leftovers first, then fd
 *    - Read the first line into _first
 *    - Move status -> READING_HEADER
 *
 * 2. READING_HEADER
 *    - Read lines until "\r\n\r\n" is found
 *    - Append lines to _head
 *    - Parse headers into _headers map
 *    - Move status -> READING_BODY
 *    - Any extra bytes after headers remain in _leftovers
 *
 * 3. READING_BODY
 *    - Consume _leftovers first, then fd
 *    - Write body bytes into _tmp_file
 *    - Stop when content-length is satisfied, EOF, or multipart boundary reached
 *
 * Other statuses (DOING, SENDING_HEAD/BODY) are not relevant here.
 *
 * Notes:
 * - _leftovers may contain extra data across reads
 * - Multipart boundaries and binary bodies are handled correctly
 * - HttpObj parsing stops at filling _headers and _tmp_file; semantic handling
 *   (CGI, file vs form field) is left to higher-level logic
 *
 * @return Current READING_STATUS or error code on failure			---*/
int		HttpObj::receive(char *buff, size_t sizeofbuff, int fd, ReadFunc reader) {
	int rtrn;

// READING_FIRST
	if (_status == HttpObj::READING_FIRST) { LOG_DEBUG("receive(): READING_FIRST")
		
		rtrn = readingFirstLine(buff, sizeofbuff, fd, reader);
		if (rtrn >= 100)
			return rtrn;
		_status = static_cast<HttpBodyStatus>(rtrn);
		if (_status == HttpObj::READING_HEADER) {
			rtrn = this->isFirstLineValid(fd);
			if (rtrn)
				return rtrn;
			LOG_INFO(printFd(fd) << "← " << _first)
		}
	}

// READING_HEADER
	if (_status == READING_HEADER) { LOG_DEBUG("receive(): READING_HEADER")

		rtrn = readingHeaders(buff, sizeofbuff, fd, reader);
		if (rtrn >= 100)
			return rtrn;
		
		_status = static_cast<HttpBodyStatus>(rtrn);
		if (_status == HttpObj::READING_BODY) {
			rtrn = parse_head_for_headers(); // check syntax
			if (rtrn)
				return rtrn;

			rtrn = this->parseHeadersForValidity();
			if (rtrn >= 100)
				return rtrn;
			_status = static_cast<HttpBodyStatus>(rtrn);
		}
	}

// READING_BODY
	if (_status == READING_BODY || _status == READING_BODY_CHUNKED) {
		if (_status == READING_BODY_CHUNKED) { LOG_DEBUG("receive(): READING_BODY_CHUNKED")
			rtrn = this->readBodyChunk(buff, sizeofbuff, fd, reader);
		}
		else if (_status == READING_BODY) { LOG_DEBUG("receive(): READING_BODY")
			rtrn = this->readBody(buff, sizeofbuff, fd, reader);
		}

		if (rtrn >= 100)
			return rtrn;
		_status = static_cast<HttpBodyStatus>(rtrn);
	}

// CLOSED
	if (_status == CLOSED) {
		LOG_INFO(printFd(fd) << "← " RED "EOF received (closed)" RESET);
	}
	
	return _status;
}

//-----------------------------------------------------------------------------]
/** @brief Reads from fd into _buffer until "\r\n" is found.
//
// - If found: store it in _first, the rest in _leftover
// , returns READING_HEADER.
// 
// - If not found: updates _bytes_received, returns READING_FIRST,
// 
// @param buff       Temporary read buffer
// @param sizeofbuff Buffer size
// @param fd         File descriptor
// @return READING_STATUS or errCode on error	---*/
int		HttpObj::readingFirstLine(char *buff, size_t sizeofbuff, int fd, ReadFunc reader) {

	const std::string	delim = "\r\n";
	StringSink			to_store_to(_first);

	ssize_t read_rtrn = readForDelim(buff, sizeofbuff, fd, delim, 0b11, to_store_to, reader);
	if(read_rtrn == -2) {
		if (_bytes_written > MAX_LIMIT_FOR_HEAD) {
			LOG_ERROR("Max Limit (" RED << MAX_LIMIT_FOR_HEAD << RESET ") reached before finding CRLF");
			return 400;
		}
		return HttpObj::READING_FIRST;
	}
	if (read_rtrn == 500)
		return 500;
	if (!read_rtrn)
		return HttpObj::CLOSED;
	if (read_rtrn == -1)
		return HttpObj::READING_FIRST;

//	read_rtrn == 1 (\r\n was found) _first now contain the first line, _leftovers the rest
	LOG_DEBUG(printFd(fd) << "→ readingFirstLine() is found: " << _first);
	return HttpObj::READING_HEADER;
}

//-----------------------------------------------------------------------------]
int		HttpObj::readingHeaders(char *buff, size_t sizeofbuff, int fd, ReadFunc reader) {

	const std::string	delim = "\r\n\r\n";
	StringSink			to_store_to(_head);

	ssize_t read_rtrn = readForDelim(buff, sizeofbuff, fd, delim, 0b11, to_store_to, reader);
	if(read_rtrn == -2) {
		if (_bytes_written > MAX_LIMIT_FOR_HEADERS) {
			LOG_ERROR("Max Limit (" RED << MAX_LIMIT_FOR_HEADERS << RESET ") reached before finding '\\r\\n\\r\\n'");
			return 400;
		}
		return HttpObj::READING_HEADER;
	}
	if (read_rtrn == 500)
		return 500;
	if (!read_rtrn)
		return HttpObj::CLOSED;
	if (read_rtrn == -1)
		return HttpObj::READING_HEADER;

//	read_rtrn == 1 (\r\n\r\n was found) _head now contain all the unparsed headers, _leftovers the rest
	LOG_DEBUG("readingHeaders() is found: " << _head);
	return HttpObj::READING_BODY;
}

///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads CGI output from a file descriptor and fills the HttpObj body.
 *
 * The function handles two main stages of CGI output:
 * 1. READING_HEADER
 *    - Reads header lines via `readingHeaders()`
 *    - On EOF or max header size, writes collected headers to _tmp_file
 *    - Parses header syntax and moves leftover bytes into _tmp_file
 *    - Updates _status accordingly
 *
 * 2. READING_BODY
 *    - Reads body bytes via `readBody()`
 *    - Stops when EOF or content-length boundary is reached
 *
 * CLOSED status indicates CGI output ended.
 *
 * @param buff Temporary buffer for reading
 * @param sizeofbuff Size of the buffer
 * @param fd File descriptor to read from
 * @return Updated HttpBodyStatus or error code on failure 	---*/
int		HttpObj::receive_cgi(char *buff, size_t sizeofbuff, int fd) {
	int rtrn;

	if (_status == READING_HEADER) { LOG_DEBUG("receive_cgi(): READING_HEADER");
		
		rtrn = readingHeaders(buff, sizeofbuff, fd, ::read);
		if (rtrn == 400 || !rtrn) { // MAX_LIMIT_FOR_HEADERS reached || EOF => no headers
			if (!_tmp_file.write(_head))
				return 500;
			_head.clear();
			_status = rtrn ? READING_BODY : CLOSED;
		}
		else if (rtrn >= 100)
			return rtrn;
		
		_status = static_cast<HttpBodyStatus>(rtrn);
		if (_status == HttpObj::READING_BODY) {
			rtrn = parse_head_for_headers(); // check syntax
			if (rtrn)
				return rtrn;
			if (!_leftovers.empty()) {
				if (!_tmp_file.write(_leftovers))
					return 500;
				_leftovers.clear();
			}
		}
	}

	if (_status == READING_BODY) { LOG_DEBUG("receive_cgi(): READING_BODY");
		rtrn = this->readBody(buff, sizeofbuff, fd, ::read);
		if (rtrn >= 100)
			return rtrn;
		_status = static_cast<HttpBodyStatus>(rtrn);
	}

	if (_status == CLOSED) {
		LOG_INFO("PIPE: " << printFd(fd) << "→ " RED "CGI finished, connection closed (EOF)" RESET);
	}
	return _status;
}

///////////////////////////////////////////////////////////////////////////////]
/***  								TOOLS									***/
///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
/** Read from fd until `delim` is found, streaming data into the given Sink.
 *
 * - If delim is found:
 *     - write up to (and optionally excluding) delim to `to_store_to`
 *     - remaining bytes go into _leftovers
 *     - _buffer is cleared
 *
 * - If delim is not found:
 *     - new data is appended to _buffer
 *
 * - _bytes_written is incremented by bytes read from fd
 *
 * @param buff        temporary buffer for recv()
 * @param sizeofbuff  size of buff
 * @param fd          file descriptor to read from
 * @param delim       delimiter string
 * @param remove_delim 0b10 remove delim in to_store_to; 0b01 remove delim in _leftovers
 * @param to_store_to Sink to store consumed bytes (string or file)
 * @return 1 if delim found, 0/-1 on recv() EOF/error, or ssize_t placeholder if not found yet (-2)
 *
 * @note careful: if remove_delim is false, decide whether delim stays in _leftovers ---*/
ssize_t		HttpObj::readForDelim(char *buff, size_t sizeofbuff, int fd, const std::string& delim, int remove_delim, Sink& to_store_to, ReadFunc reader) {

// first check _leftovers
	int rtrn;
	if((rtrn = findDelimInLeftovers(delim, remove_delim, to_store_to)) != 0)
		return rtrn;

// add the last bytes of _buffer for delim split over recv()
	size_t n = _buffer.size() >= delim.size() ?  _buffer.size() - delim.size() + 1 : 0;

// append the return of recv() to _buffer
	ssize_t bytes_recv = readBuffer(buff, sizeofbuff, fd, _buffer, reader);
	if (bytes_recv <= 0)
		return bytes_recv; // 0 = EOF / close connection; -1 = error or EAGAIN
	_bytes_written += bytes_recv;

	size_t pos = _buffer.find(delim, n);
	if (pos == std::string::npos)
		return -2;
	
// if found, does the splitting
	if (!to_store_to.write(_buffer.c_str(), pos + delim.size() * !(remove_delim & 0b10))) {
		LOG_ERROR_SYS(printFd(fd) << "→ readForDelim(): partial write() detected");
		return 500;
	}
	_leftovers = _buffer.substr(pos + delim.size() * (remove_delim & 0b01));
	_buffer.clear();
	
	return 1;
}

//-----------------------------------------------------------------------------]
/**	Scan _leftovers for `delim` and move data to the provided Sink.
 *
 * - If `delim` is found:
 *     - write pre-delim to `to_store_to` (+- delim included)
 *     - erase consumed bytes from _leftovers
 * - If not found:
 *     - copy all of _leftovers to _buffer
 *     - clear _leftovers
 *
 * @param delim        delimiter to search for
 * @param remove_delim 0b10 remove delim in to_store_to; 0b01 remove delim in _leftovers
 * @param to_store_to  destination Sink for consumed bytes (string | fd_file)
 * @return 1 if delimiter found, 0 if not found, 500 on write error
 *
 * @note after succesfull call: (_leftovers can be empty if delim was found at npos) ---*/
int		HttpObj::findDelimInLeftovers(const std::string& delim, int remove_delim, Sink& to_store_to) {

	if (_leftovers.empty())
		return 0;
	
// if(true); //dont erase delim, either end of buffer, or beginnning of leftover 
	size_t pos = _leftovers.find(delim);
	if (pos == std::string::npos) {
			_buffer = _leftovers;
			_leftovers.clear();
			return 0;
	}
	else {
		if (!to_store_to.write(_leftovers.c_str(), pos + delim.size() * !(remove_delim & 0b10))) {
			LOG_ERROR_SYS("→ findDelimInLeftovers(): partial write() detected");
			return 500;
		}
		_leftovers.erase(0, pos + delim.size() * (remove_delim & 0b01));
		return 1;
	}
}
