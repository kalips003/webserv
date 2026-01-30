#include "HttpObj.hpp"
#include "Log.hpp"

#include <algorithm>
#include <netinet/in.h>
#include <string.h>

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
/***  								READ									***/
///////////////////////////////////////////////////////////////////////////////]

//-----------------------------------------------------------------------------]
/** @brief Reads from fd and appends them to a string.
 *
 * @return The return of recv()		---*/
ssize_t HttpObj::readBuffer(char *buff, size_t sizeofbuff, int fd, std::string& to_append_to) {

	ssize_t bytes_recv = recv(fd, buff, sizeofbuff - 1, 0);

	if (bytes_recv <= 0)
		return bytes_recv;

	LOG_INFO(printFd(fd) << C_134 " packet received (" RESET << bytes_recv << C_134 " bytes)" RESET);
	to_append_to.append(buff, bytes_recv);

	return bytes_recv;
}

///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from fd into _tmp_file.
//
* @return READING_STATUS or errCode on error		---*/
// int		HttpObj::readingBody(char *buff, size_t sizeofbuff, int fd) {
int    HttpObj::streamingBody(char *buff, size_t sizeofbuff, int fd) {

	ssize_t bytes_recv = recv(fd, buff, sizeofbuff, 0);

	if (!bytes_recv)
		return HttpObj::CLOSED;
	else if (bytes_recv < 0)
		return HttpObj::READING_BODY;

	LOG_INFO(printFd(fd) << C_134 " packet received (" RESET << bytes_recv << C_134 " bytes)" RESET);
	
	ssize_t rtrn_write;
	if ((rtrn_write = write(_tmp_file._fd, buff, bytes_recv)) < bytes_recv) {
		LOG_ERROR_SYS("streamingBody(): write(): partial write");
		return 500;
	}
	_bytes_written += bytes_recv;

	if (_bytes_written >= static_cast<size_t>(_bytes_total))
		return HttpObj::DOING;
	return HttpObj::READING_BODY;
}

///////////////////////////////////////////////////////////////////////////////]
/** Read from fd until `delim` is found, streaming data into the given Sink.
 *
 * - If delim is found:
 *     - write up to (and optionally excluding) delim to `to_store_to`
 *     - remaining bytes go into _leftovers
 *     - _buffer is cleared
 * - If delim is not found:
 *     - new data is appended to _buffer
 *
 * - _bytes_written is incremented by bytes read from fd
 *
 * @param buff        temporary buffer for recv()
 * @param sizeofbuff  size of buff
 * @param fd          file descriptor to read from
 * @param delim       delimiter string
 * @param remove_delim if true, delim is removed and not written
 * @param to_store_to Sink to store consumed bytes (string or file)
 * @return 1 if delim found, 0/-1 on recv() EOF/error, or ssize_t placeholder if not found yet (-2)
 *
 * @note careful: if remove_delim is false, decide whether delim stays in _leftovers ---*/
ssize_t		HttpObj::readForDelim(char *buff, size_t sizeofbuff, int fd, const std::string& delim, bool remove_delim, Sink& to_store_to) {

// first check _leftovers
	int rtrn;
	if((rtrn = findDelimInLeftovers(delim, remove_delim, to_store_to)) != 0)
		return rtrn;

// add the last bytes of _buffer for delim split over recv()
	size_t n = _buffer.size() >= delim.size() ?  _buffer.size() - delim.size() + 1 : 0;

// append the return of recv() to _buffer
	ssize_t bytes_recv = readBuffer(buff, sizeofbuff, fd, _buffer);
	if (bytes_recv <= 0)
		return bytes_recv; // 0 = EOF / close connection; -1 = error or EAGAIN
	_bytes_written += bytes_recv;

	size_t pos = _buffer.find(delim, n);
	if (pos == std::string::npos)
		return -2;

// if found, does the splitting
	if (!to_store_to.write(_buffer.c_str(), pos + (remove_delim ? 0 : delim.size()))) {
		LOG_ERROR_SYS(printFd(fd) << "→ readForDelim(): partial write() detected");
		return 500;
	}
	_leftovers = _buffer.substr(pos + (remove_delim ? delim.size() : 0));
	_buffer.clear();
	
	return 1;
}

///////////////////////////////////////////////////////////////////////////////]
/***  								SEND									***/
///////////////////////////////////////////////////////////////////////////////]
// #include <string.h>
//-----------------------------------------------------------------------------]
/** @brief Sends from a string over fd.
 * *
 * Updates the string by removing the bytes that were sent.
 *
 * @return The return of send()		---*/
ssize_t	HttpObj::sendBufferString(char *buff, size_t sizeofbuff, int fd, std::string& to_send_from) {

	ssize_t bytesLoaded = std::min(sizeofbuff, to_send_from.size());
	memcpy(buff, to_send_from.c_str(), bytesLoaded);

	ssize_t bytesSent = ::send(fd, buff, bytesLoaded, 0);

	if (bytesSent <= 0)
		return bytesSent;

oss msg; msg << "[#" << printFd(fd) << "] " << C_134 "packet sent (" RESET << bytesSent << C_134 " bytes)" RESET; printLog(INFO, msg.str(), 1);
// msg.str(""); msg << C_134 "Packet: [" RESET << to_send_from.substr(0, bytesSent) << C_134 "]" RESET;printLog(DEBUG, msg.str(), 1);

	to_send_from.erase(0, bytesSent);

	return bytesSent;
}

//-----------------------------------------------------------------------------]
/** @brief Sends from a string over fd.
 * *
 * Updates the string by removing the bytes that were sent.
 *
 * @return The return of send()		---*/
ssize_t	HttpObj::sendBufferFile(char *buff, size_t sizeofbuff, int fd, int fd_file) {

	ssize_t bytesLoaded = read(fd_file, buff, sizeofbuff);
	if (bytesLoaded < 0) {
		printErr("sendBufferFile(): read()");
		return bytesLoaded;
	}

	ssize_t bytesSent = ::send(fd, buff, bytesLoaded, 0);

	if (bytesSent <= 0)
		return bytesSent;

	if (bytesSent < bytesLoaded) {
		_leftovers.append(buff, bytesSent);
		LOG_INFO(printFd(fd) << C_134 "partial packet sent (loaded: " RESET << bytesLoaded << C_134 ", sent: " RESET << bytesSent << C_134 " bytes)" RESET);
	}
	else
		LOG_INFO(printFd(fd) << C_134 "packet sent (" RESET << bytesSent << C_134 " bytes)" RESET);

	LOG_DEBUG(C_134 "Packet: [" RESET << std::string(buff, bytesSent) << C_134 "]" RESET);

	return bytesSent;
}