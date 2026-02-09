#include "HttpRequest.hpp"

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
/*
	Transfer-Encoding: chunked
	
	<hex-size>\r\n
	<data>\r\n
	<hex-size>\r\n
	<data>\r\n
	0\r\n
	\r\n
 */
ssize_t		readHexa(const std::string& hexa);
#define MAX_HEXA_LINE_LENGTH 32
///////////////////////////////////////////////////////////////////////////////]
int		HttpRequest::readBodyChunk(char *buff, size_t sizeofbuff, int fd, ReadFunc reader) {
// LOG_LOG("readBodyChunk()")

	if (_bytes_total != -1) {
		_leftovers = _buffer;
		_buffer.clear();
	}
	int rtrn;

	while (true) {

		if (_bytes_total < 0) {// -1 search for <hex>\r\n
			rtrn = readBodyChunk_delimHelper(buff, sizeofbuff, fd, reader);
		// error // EOF // try again (recv = -1); all in _buffer
			if (rtrn >= 100 || rtrn == HttpObj::CLOSED || rtrn == HttpObj::READING_BODY_CHUNKED)
				break;
			if (rtrn == -2) // <hex>\r\n not found yet, retry; all in _buffer
				return  HttpObj::READING_BODY_CHUNKED;
			if (rtrn == HttpObj::FINISHED) // we are done here, last chunk was found
				return HttpObj::DOING;
		}

		if (_bytes_total) {// <size> stream for size
			rtrn = streamingBodyWrapper(buff, sizeofbuff, fd, reader);
		// error // EOF // after one call to recv, we still dont have everything, need one more epoll call
			if (rtrn >= 100 || rtrn == HttpObj::CLOSED || rtrn == HttpObj::READING_BODY_CHUNKED) // error
				break;
			if (rtrn == HttpObj::FINISHED) // previous part was found, _bytes reset; all in _leftovers
				continue;
		}
	}

	return rtrn;
}

///////////////////////////////////////////////////////////////////////////////]
int		HttpRequest::readBodyChunk_delimHelper(char *buff, size_t sizeofbuff, int fd, ReadFunc reader) {

	StringSink			to_store_to(_body);
	const std::string	delim = "\r\n";

	ssize_t read_rtrn = readForDelim(buff, sizeofbuff, fd, delim, 0b11, to_store_to, reader);
	if (_bytes_total == -2) { // -2 look for \r\n<hex\r\n
		if (!_body.empty()) // not CRLF
			return 400;
		_bytes_total = -1;
		read_rtrn = readForDelim(buff, sizeofbuff, fd, delim, 0b11, to_store_to, reader);
	}
// error handling
	if (read_rtrn >= 100 )
		return read_rtrn;
	if (!read_rtrn)
		return HttpObj::CLOSED; // EOF found
	if (read_rtrn == -1)
		return HttpObj::READING_BODY_CHUNKED; // try again (recv = -1)
// not found yet
	if (read_rtrn == -2) {
		if (_buffer.size() > MAX_HEXA_LINE_LENGTH) { // ??? infinity check?
			LOG_ERROR("readBodyChunk(): Limit (" << MAX_HEXA_LINE_LENGTH << ") reached before finding chunk size: " << _buffer)
			return 400;
		}
		return read_rtrn; // all in _buffer
	}
// else found: (return 1)
	_bytes_total = readHexa(_body);
	if (_bytes_total < 0) {
		LOG_ERROR("readBodyChunk(): Invalid chunk size: (" << _bytes_total << ") body: (" << _body << ")")
		return 400;
	}
	if (_bytes_total == 0) // we are done here, last chunk was found
		return HttpObj::FINISHED;
	_body.clear();
	_bytes_written = 0;
	return HttpObj::DOING;
}

///////////////////////////////////////////////////////////////////////////////]
int		HttpRequest::streamingBodyWrapper(char *buff, size_t sizeofbuff, int fd, ReadFunc reader) {
// LOG_LOG("streamingBodyWrapper()")
// first check leftovers
	if (_leftovers.size()) {
		size_t to_write = std::min(_leftovers.size(), static_cast<size_t>(_bytes_total) - _bytes_written);
		ssize_t write_rtrn = write(_tmp_file._fd, _leftovers.c_str(), to_write);
		if (write_rtrn < 0 || static_cast<size_t>(write_rtrn) < to_write)
			return 500;
		_leftovers.erase(0, to_write);
		_bytes_written += to_write;
		if (_bytes_written >= static_cast<size_t>(_bytes_total)) { // reset counters for next chunk
			_bytes_total = -2;
			_bytes_written = 0;
			return HttpObj::FINISHED;
		}
	}
// next try a read
	int rtrn = streamingBody(buff, sizeofbuff, fd, reader);
	if (rtrn == HttpObj::DOING) {
		_bytes_total = -2;
		_bytes_written = 0;
		return HttpObj::FINISHED;
	}
	else if (rtrn == HttpObj::READING_BODY)
		return HttpObj::READING_BODY_CHUNKED;
	return rtrn;
}

///////////////////////////////////////////////////////////////////////////////]
ssize_t		readHexa(const std::string& hexa) {

	if (hexa.empty())
		return -1; // invalid

	size_t chunk_size = 0;
	std::stringstream ss(hexa);
	ss >> std::hex >> chunk_size;

	if (ss.fail())
		return -1; // invalid hex

	return static_cast<ssize_t>(chunk_size);
}

///////////////////////////////////////////////////////////////////////////////]
int		HttpRequest::detectChunkedEncoding() {

	const std::string* chunk = find_in_headers("Transfer-Encoding");
	if (!chunk)
		return HttpObj::DOING;
	if (*chunk != "chunked")
		return 501; // Not Implemented (or 400)
	if (!(_method == "POST" || _method == "PUT" || _method == "PATCH"))
		return 400;

	if (!_tmp_file.createTempFile(&g_settings.getTempRoot()))
		return 500;

	_bytes_total = -1;
	_bytes_written = 0;
	return HttpObj::READING_BODY_CHUNKED;
}
