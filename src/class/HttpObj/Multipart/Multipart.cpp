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
int		HttpMultipart::parse_multifile(char *buff, size_t sizeofbuff, int fd) {

	int rtrn;

	if (_status == HttpObj::READING_FIRST) { LOG_DEBUG("parse_multifile(): READING_FIRST");
		
		rtrn = readingFirstLine(buff, sizeofbuff, fd);
		if (rtrn >= 100)
			return rtrn;
		LOG_ERROR("rtrn = " << rtrn << "; _delim = " << _delim);
		LOG_INFO("_first = {" << _first << "}");
		LOG_LOG("_buffer = {" << _buffer << "}");

		_status = static_cast<HttpBodyStatus>(rtrn);
		if (_status == HttpObj::READING_HEADER) {
			rtrn = this->isFirstLineValid(fd);
			if (rtrn)
				return rtrn;
		}
	}

	if (_status == READING_HEADER) { LOG_DEBUG("parse_multifile(): READING_HEADER");
		
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
			if (!_tmp_file.createTempFile(&g_settings.getTempRoot()))
				return 500;
		}
	}

	if (_status == READING_BODY) { LOG_DEBUG("parse_multifile(): READING_BODY");

		rtrn = readingBody(buff, sizeofbuff, fd);
		if (rtrn >= 100)
			return rtrn;
		_status = static_cast<HttpBodyStatus>(rtrn);
	}

	if (_status == CLOSED) { LOG_DEBUG("parse_multifile(): CLOSED");
		LOG_INFO(printFd(fd) << "→ " RED "READ closed (EOF)" RESET);
	}
	
	return _status;
}


//-----------------------------------------------------------------------------]
/**
// _bytes_total should have the value of other._body.size
// _bytes_written get incremented, never reset */
int		HttpMultipart::readingBody(char *buff, size_t sizeofbuff, int fd) {

	StringSink			to_store_to(_buffer);

	ssize_t read_rtrn = readForDelim(buff, sizeofbuff, fd, "\r\n" + _delim, true, to_store_to); // << keep boundary
	if(read_rtrn == -2) { // not found yet
		if (_bytes_written > static_cast<size_t>(_bytes_total)) {
			LOG_WARNING("Tempfile size (" RED << _bytes_total << RESET ") reached before finding: " << _delim)
			return 400;
		}
	
		ssize_t rtrn_write;
		size_t to_write_to_file = _buffer.size() >= _delim.size() + 2 ? _buffer.size() - _delim.size() - 1 : 0;
		if (static_cast<size_t>(rtrn_write = write(_tmp_file._fd, _buffer.c_str(), to_write_to_file)) < to_write_to_file) {
			LOG_ERROR_SYS("readingBody(): write(): partial write");
			return 500;
		}
		_buffer.erase(0, to_write_to_file);
		return HttpObj::READING_BODY;
	}
	if (read_rtrn == 500)
		return 500;
	if (!read_rtrn)
		return HttpObj::CLOSED; // EOF found
	if (read_rtrn == -1)
		return HttpObj::READING_BODY;

//	read_rtrn == 1 (--BOUNDARY was found) _buffer the last read, _leftovers the rest
	// check next 2 char , we only found \r\n--DELIM..
	// check for \r\n or -- or 400

	LOG_DEBUG("readingBody() is found: " << _buffer);
	return HttpObj::DOING;
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

	return 0;
}

// #include "SettingsServer.hpp"
///////////////////////////////////////////////////////////////////////////////]
HttpMultipart::HttpMultipart(const HttpMultipart& other) : HttpObj(), _delim(other._delim) {
	_leftovers = other._leftovers;
	_bytes_total = other._bytes_total;
	_bytes_written = other._bytes_written;
	_status = HttpObj::READING_FIRST;
}

