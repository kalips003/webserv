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
ssize_t HttpObj::readBuffer(char *buff, size_t sizeofbuff, int fd, std::string& to_append_to, ReadFunc reader) {

	ssize_t bytes_recv = reader(fd, buff, sizeofbuff);

	if (bytes_recv <= 0)
		return bytes_recv;

	LOG_INFO(printFd(fd) << C_134 "packet received (" RESET << bytes_recv << C_134 " bytes)" RESET);
	to_append_to.append(buff, bytes_recv);

	return bytes_recv;
}

///////////////////////////////////////////////////////////////////////////////]
/** @brief Reads from fd into _tmp_file.
//
* @return READING_STATUS or errCode on error		---*/
// int		HttpObj::readingBody(char *buff, size_t sizeofbuff, int fd) {
int    HttpObj::streamingBody(char *buff, size_t sizeofbuff, int fd, ReadFunc reader) {

	ssize_t bytes_recv = reader(fd, buff, sizeofbuff);
	LOG_DEBUG("-> streamingBody, bytes_recv = " << bytes_recv)
	if (!bytes_recv)
		return HttpObj::CLOSED;
	else if (bytes_recv < 0)
		return HttpObj::READING_BODY;

	LOG_INFO(printFd(fd) << C_134 "packet received (" RESET << bytes_recv << C_134 " bytes)" RESET);
	
	ssize_t rtrn_write;
	if ((rtrn_write = write(_tmp_file._fd, buff, bytes_recv)) < bytes_recv) {
		LOG_ERROR_SYS("streamingBody(): write(): partial write");
		return 500;
	}
	_bytes_written += bytes_recv;
	LOG_DEBUG("streamingBody(): _bytes_total= " << _bytes_total << " _bytes_written = " << _bytes_written)
	if (_bytes_written >= static_cast<size_t>(_bytes_total))
		return HttpObj::DOING;
	return HttpObj::READING_BODY;
}

//-----------------------------------------------------------------------------]
ssize_t recv0(int fd, void* buf, size_t n) {
	return recv(fd, buf, n, 0);
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

	LOG_INFO(printFd(fd) << C_134 "packet sent (" RESET << bytesSent << C_134 " bytes)" RESET);
	LOG_DEBUG(printFd(fd) << C_134 "packet sent (from string) (" RESET << bytesSent << C_134 " bytes): {" RESET << std::string(buff, bytesSent));
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
	else {
		LOG_INFO(printFd(fd) << C_134 "packet sent (" RESET << bytesSent << C_134 " bytes)" RESET);
		LOG_DEBUG(printFd(fd) << C_134 "packet sent (from tmp_file) (" RESET << bytesSent << C_134 " bytes): {" RESET << std::string(buff, bytesSent));
	}

	LOG_DEBUG(C_134 "Packet: [" RESET << std::string(buff, bytesSent) << C_134 "]" RESET);

	return bytesSent;
}