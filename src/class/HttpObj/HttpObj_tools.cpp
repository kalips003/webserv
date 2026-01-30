#include "HttpObj.hpp"

#include "Log.hpp"
#include "Tools1.hpp"

#include <netinet/in.h>
#include <string.h>

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

LOG_INFO("[#" << printFd(fd) << "] " << C_134 "packet received (" RESET << bytes_recv << C_134 " bytes)" RESET);
LOG_DEBUG(C_134 "Packet: [" RESET << std::string(buff, bytes_recv) << C_134 "]" RESET);

	to_append_to.append(buff, bytes_recv);

	return bytes_recv;
}

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
ssize_t HttpObj::read_until_delim_is_found(char *buff, size_t sizeofbuff, int fd, const std::string& delimitor, bool& is_found) {

// copy the last bytes of buff_to_append
	size_t n = _buffer.size() > delimitor.size() ? delimitor.size() - 1 : _buffer.size();

// append the return of recv()
	ssize_t bytes_recv = readBuffer(buff, sizeofbuff, fd, _buffer);
	if (bytes_recv <= 0)
		return bytes_recv;

	size_t pos = _buffer.find(delimitor, n);
	if (pos == std::string::npos)
		return bytes_recv;

// if found, does the splitting
	_leftovers = _buffer.substr(pos);
	_buffer.resize(pos);
	is_found = true;
	
	return bytes_recv;
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

LOG_INFO("[#" << printFd(fd) << "] " << C_134 "packet sent (" RESET << bytesSent << C_134 " bytes)" RESET);
LOG_DEBUG(C_134 "Packet: [" RESET << to_send_from.substr(0, bytesSent) << C_134 "]" RESET);

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
		LOG_LOG("[#" << printFd(fd) << "] " << C_134 "partial packet sent (loaded: " RESET << bytesLoaded << C_134 ", sent: " RESET << bytesSent << C_134 " bytes)" RESET);
	}
	else {
		LOG_INFO("[#" << printFd(fd) << "] " << C_134 "packet sent (" RESET << bytesSent << C_134 " bytes)" RESET);
	} 

LOG_DEBUG(C_134 "Packet: [" RESET << std::string(buff, bytesSent) << C_134 "]" RESET);

	return bytesSent;
}
