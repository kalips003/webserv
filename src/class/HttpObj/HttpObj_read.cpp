#include "HttpObj.hpp"

#include "Tools1.hpp"

#include <string.h>

///////////////////////////////////////////////////////////////////////////////]
/***  								READ									***/
///////////////////////////////////////////////////////////////////////////////]
/** HttpObj::receive mapping / state machine
 *
 * This function reads from a file descriptor (fd) and a leftover buffer
 * to fill the HttpObj instance, handling both full HTTP requests and
 * multipart/form-data parts.
 *
 * State transitions:
 *
 * 1. READING_FIRST
 *    - Consume _leftovers first, then fd
 *    - Read until first "\r\n" is found
 *    - Store the line in _first
 *    - Move _status -> READING_HEADER
 *    - Edge cases:
 *        * _leftovers may already contain more than one line
 *        * "\r\n" may be split across reads
 *
 * 2. READING_HEADER
 *    - Read lines until "\r\n\r\n" is found
 *    - Append lines to _head
 *    - Parse headers into _headers map
 *    - Move _status -> READING_BODY
 *    - Edge cases:
 *        * headers may span multiple reads
 *        * extra bytes after "\r\n\r\n" remain in _leftovers for body
 *
 * 3. READING_BODY
 *    - Consume _leftovers first, then fd
 *    - Write body bytes into _tmp_file
 *    - Update _bytes_written accordingly
 *    - Stop when:
 *        a) _bytes_total is known and _bytes_written >= _bytes_total
 *        b) EOF on fd
 *        c) (for multipart) boundary delimiter is reached
 *    - Body is treated as binary-safe; do not assume null-terminated strings
 *    - Edge cases:
 *        * _leftovers may contain start of next part
 *        * multipart parts do not have Content-Length; must detect boundary
 *
 * 4. Other statuses (DOING, SENDING_HEAD, SENDING_BODY, SENDING_BODY_FD)
 *    - Not relevant for parsing; reserved for sending or application logic
 *
 * Notes:
 * - HttpObj is a dumb container: it stores headers, first line, body, and temp file
 * - Responsibility of parsing ends at filling _headers and _tmp_file
 * - Semantic interpretation (file vs regular field, CGI handling) is left to
 *   higher-level logic, e.g., Ft_Post or application layer	---*/

///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from fd.
// internally update status
// 		read firt line into _first
// 		read headers into _head
// 		read body into _tmp_file
//
* @return READING_STATUS or errCode on error		---*/
int		HttpObj::receive_request(char *buff, size_t sizeofbuff, int fd) {
	int rtrn;

	if (_status == HttpObj::READING_FIRST) { LOG_DEBUG("receive_request(): READING_FIRST");
		
		rtrn = readingFirstLine(buff, sizeofbuff, fd);
		if (rtrn >= 100)
			return rtrn;

		_status = static_cast<HttpBodyStatus>(rtrn);
		if (_status == HttpObj::READING_HEADER) {
			rtrn = this->isFirstLineValid(fd);
			if (rtrn)
				return rtrn;
		}
	}

	if (_status == READING_HEADER) { LOG_DEBUG("receive_request(): READING_HEADER");
		
		rtrn = readingHeaders(buff, sizeofbuff, fd);
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

	if (_status == READING_BODY) { LOG_DEBUG("receive_request(): READING_BODY");

		rtrn = streamingBody(buff, sizeofbuff, fd);
		if (rtrn >= 100)
			return rtrn;
		_status = static_cast<HttpBodyStatus>(rtrn);
	}

	if (_status == CLOSED) {
		LOG_DEBUG("receive(): CLOSED")
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
int		HttpObj::readingFirstLine(char *buff, size_t sizeofbuff, int fd) {

	const std::string	delim = "\r\n";
	StringSink			to_store_to(_first);

	ssize_t read_rtrn = readForDelim(buff, sizeofbuff, fd, delim, true, to_store_to);
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
int		HttpObj::readingHeaders(char *buff, size_t sizeofbuff, int fd) {

	const std::string	delim = "\r\n\r\n";
	StringSink			to_store_to(_head);

	ssize_t read_rtrn = readForDelim(buff, sizeofbuff, fd, delim, true, to_store_to);
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
/***  								TOOLS									***/
///////////////////////////////////////////////////////////////////////////////]

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
 * @param remove_delim if true, delimiter is erased
 * @param to_store_to  destination Sink for consumed bytes (string | fd_file)
 * @return 1 if delimiter found, 0 if not found, 500 on write error
 *
 * @note after succesfull call: (_leftovers can be empty if delim was found at npos) ---*/
int		HttpObj::findDelimInLeftovers(const std::string& delim, bool remove_delim, Sink& to_store_to) {

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
		if (!to_store_to.write(_leftovers.c_str(), pos + (remove_delim ? delim.size() : 0))) {
				LOG_ERROR_SYS("findDelimInLeftovers(): partial write() detected");
			return 500;
		}
		size_t pos_cut = remove_delim ? pos + delim.size() : pos;
		_leftovers.erase(0, pos_cut);
		return 1;
	}
}
