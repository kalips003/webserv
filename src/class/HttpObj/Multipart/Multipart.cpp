#include "Multipart.hpp"

// #include <iostream>
// #include <algorithm>

#include "Tools1.hpp"
// #include "HttpMethods.hpp"
#include "SettingsServer.hpp"
#include <cstddef>

///////////////////////////////////////////////////////////////////////////////]
/*
	--BOUNDARY\r\n
	<part headers>\r\n
	\r\n
	<part body>
	\r\n
	--BOUNDARY\r\n
	<part headers>\r\n
	\r\n
	<part body>
	\r\n
	--BOUNDARY--\r\n
 */
/**	@return HttpObj::DOING if finished finding last delim, CLOSED on EOF */
//-----------------------------------------------------------------------------]
/**
// _bytes_total should have the value of other._body.size
// _bytes_written get incremented, never reset */
int		HttpMultipart::readingBody(char *buff, size_t sizeofbuff, int fd, ReadFunc reader) {

	StringSink			to_store_to(_buffer);

	ssize_t read_rtrn = readForDelim(buff, sizeofbuff, fd, "\r\n" + _delim, 0b10, to_store_to, reader); // << keep boundary in _leftovers
	if (read_rtrn == 500)
		return 500;
	if (!read_rtrn)
		return HttpObj::CLOSED; // EOF found
	if (read_rtrn == -1)
		return HttpObj::READING_BODY;
	
	size_t to_write_to_file;
	if (read_rtrn == -2) { // not found yet
		if (_bytes_written > static_cast<size_t>(_bytes_total)) {
			LOG_WARNING("Tempfile size (" RED << _bytes_total << RESET ") reached before finding: " << _delim)
			return 400;
		}
		to_write_to_file = _buffer.size() >= _delim.size() + 2 ? _buffer.size() - _delim.size() - 1 : 0;
	}
	else // found
		to_write_to_file = _buffer.size();

	ssize_t rtrn_write;
	if (static_cast<size_t>(rtrn_write = write(_tmp_file._fd, _buffer.c_str(), to_write_to_file)) < to_write_to_file) {
		LOG_ERROR_SYS("readingBody(): write(): partial write");
		return 500;
	}
	// LOG_DEBUG("readingBody() is found: _buffer before: " << _buffer);
	_buffer.erase(0, to_write_to_file);

	if (read_rtrn == -2)
		return HttpObj::READING_BODY;
//	read_rtrn == 1 (r\n--BOUNDARY was found) written to to_store_to, _leftovers the rest (boundary included)
	// LOG_DEBUG("readingBody() is found: _buffer: " << _buffer);
	// LOG_DEBUG("readingBody() is found: _leftovers: " << _leftovers);
	return HttpObj::DOING;
}

/*
	--BOUNDARY\r\n
	<part headers>\r\n
	\r\n
	<part body>
	\r\n
	--BOUNDARY\r\n
	<part headers>\r\n
	\r\n
	<part body>
	\r\n
	--BOUNDARY--\r\n
 */
///////////////////////////////////////////////////////////////////////////////]
/** Checks the next two bytes after a multipart found its boundary to check if
 * final "--", continue "\r\n", or malformed request (400)
 * 
 * Reads from _leftovers or the underlying fd if necessary.
 * Consumes the leading "\r\n" before the boundary and updates _bytes_written.
 *
 * @return - HttpObj::DOING  : a regular boundary was found, more parts follow
 * @return - HttpObj::CLOSED : final boundary ("--") found, optionally allowing a trailing CRLF
 * @return - 400             : invalid multipart formatting or unexpected data 	---*/
int		HttpMultipart::tool_check_next_two_char(int fd) {

	char end[3] = "..";
	ssize_t stuff_after_delim = _leftovers.size() - (_delim.size() + 2);
// LOG_HERE(RED "0.3: _leftovers: {" RESET << _leftovers <<RED "}\n" RESET "stuff_after_delim: " << stuff_after_delim)
	if (stuff_after_delim >= 2) {// [\r\n + DELIM + ".."]
		end[0] = _leftovers[_delim.size() + 2];
		end[1] = _leftovers[_delim.size() + 3];
	}
	else {
		ssize_t read_rtrn = read(fd, end + stuff_after_delim, 2 - stuff_after_delim);
// LOG_HERE(RED "0.3: _leftovers: {" RESET << _leftovers <<RED "}\n" RESET "stuff_after_delim: " << stuff_after_delim)
		if (read_rtrn < 2 - stuff_after_delim)
			return 400;
		if (stuff_after_delim == 1)
			end[0] = _leftovers[_delim.size() + 2];
		_bytes_written += read_rtrn;
	}
	_leftovers.erase(0, 2); // erase first '\r\n' in previsions for the next HttpMultipart

	if (end[0] == '-' && end[1] == '-') {
		if (_bytes_written != static_cast<size_t>(_bytes_total)) {// garbage after, last '\r\n' can exist
			if (static_cast<size_t>(_bytes_total) == _bytes_written + 2 && tool_check_next_two_char(fd) == HttpObj::DOING)
				return HttpObj::CLOSED;
			return 400;
		}
		return HttpObj::CLOSED;
	}
	if (end[0] == '\r' && end[1] == '\n')
		return HttpObj::DOING;
	return 400;
}


///////////////////////////////////////////////////////////////////////////////]
int		HttpMultipart::isFirstLineValid(int fd) { 
	(void)fd;
	return _first == _delim ? 0 : 400;
}

///////////////////////////////////////////////////////////////////////////////]
int		HttpMultipart::parseHeadersForValidity() {

	std::string* content_disposition = find_in_headers("Content-Disposition");
	if (!content_disposition)
		return 400;

// Content-Disposition: form-data; name="foo"; filename="bar.txt"
	std::vector<std::string> v = splitOnDelimitor(*content_disposition, ";");
	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {

		std::string param = trim_white(*it);
		*it = param;
		if (param.find("name=") == 0)
			_name = trim_any(param.substr(5), " \"\'"); // mandatory
		else if (param.find("filename=") == 0)
			_filename = trim_any(param.substr(9), " \"\'"); // optional
	}
	if (v.empty() || v[0] != "form-data" || _name.empty())
		return 400;

	if (!_tmp_file.createTempFile(&g_settings.getTempRoot()))
		return 500;
	return HttpObj::READING_BODY;
}

// #include "SettingsServer.hpp"
///////////////////////////////////////////////////////////////////////////////]
HttpMultipart::HttpMultipart(const HttpMultipart& other) : HttpObj(), _delim(other._delim) {
	_leftovers = other._leftovers;
	_bytes_total = other._bytes_total;
	_bytes_written = other._bytes_written;
	_status = HttpObj::READING_FIRST;
}

