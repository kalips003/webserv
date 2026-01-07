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
		std::cerr << RED "connection closed (FIN received)" RESET << std::endl;
		return CLOSED;
	}
	else if (bytes_recv < 0) {// treat as generic fail (no errno)
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return _status; // fcntl()'s fault, no data to read yet
		printErr(RED "recv() failed" RESET);
		return CLOSED; //?
		// else if (errno == EINTR)
		//	 return -2; // interrupted by signal, retry
	}

	buff[bytes_recv] = '\0';
	std::string str_buff(buff, bytes_recv);
	std::cerr << C_134 "packet received (" RESET << bytes_recv << C_134 " bytes): \n[" RESET << str_buff << C_134 "]" RESET << std::endl;

	int rtrn;
    if (_status == FIRST) {
		rtrn = _request.readingFirstLine(str_buff);
		if (rtrn >= 100)
			return _answer.create_error(rtrn);
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

