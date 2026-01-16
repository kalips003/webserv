#include "Connection.hpp"
#include "_colors.h"

#include <iostream>

#include "Tools1.hpp"
#include <errno.h>
///////////////////////////////////////////////////////////////////////////////]
/** Read from provided buffer once
 *
 * transform it into a string, pass it to _request for handling, based on _status
 * _request update internally the _Request object
 *
 * @param buff   Buffer to read from
 * @param sizeofbuff   Size of said buffer
 * @return      ConnectionStatus of the state of the connection after this one read
 *
 * in case of error, fills the _answer with the error, and return SENDING	---*/
enum ConnectionStatus Connection::ft_read(char *buff, size_t sizeofbuff) {

	ssize_t bytes_recv = recv(_client_fd, buff, sizeofbuff - 1, 0);

	if (bytes_recv == 0) {
		printLog(INFO, RED "connection closed (FIN received)" RESET, 1);
		return CLOSED;
	}
	else if (bytes_recv < 0) {// treat as generic fail (no errno)
		printLog(WARNING, "bytes_recv: -1", 1);
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return _status; // fcntl()'s fault, no data to read yet
		printErr(RED "recv() failed" RESET);
		return CLOSED; //?
		// else if (errno == EINTR)
		//	 return -2; // interrupted by signal, retry
	}

	buff[bytes_recv] = '\0';
	std::string str_buff(buff, bytes_recv);

	oss msg; msg << "[#" C_431 << _client_fd << RESET "] " << C_134 "packet received (" RESET << bytes_recv << C_134 " bytes)" RESET;
	printLog(INFO, msg.str(), 1);
	msg.str(""); msg << C_134 "Packet: [" RESET << str_buff << C_134 "]" RESET;
	printLog(DEBUG, msg.str(), 1);

	int rtrn;
    if (_status == FIRST) {
		rtrn = _request.readingFirstLine(str_buff);
		if (rtrn >= 100)
			return _answer.create_error(rtrn);
		if (rtrn == READING_HEADER) {
			oss msg; msg << "[#" C_431 << _client_fd << RESET "] → " << _request.getMethod() << " " << _request.getPath();
			printLog(INFO, msg.str(), 1);
		}
		_status = static_cast<ConnectionStatus>(rtrn);
	}

	if (_status == READING_HEADER) {
		rtrn = _request.readingHeaders(str_buff);
		if (rtrn >= 100)
			return _answer.create_error(rtrn);
		_status = static_cast<ConnectionStatus>(rtrn);
	}

	if (_status == READING_BODY) {
		rtrn = _request.readingBody(str_buff);
		if (rtrn >= 100)
			return _answer.create_error(rtrn);
		_status = static_cast<ConnectionStatus>(rtrn);
	}

	return _status;
}

