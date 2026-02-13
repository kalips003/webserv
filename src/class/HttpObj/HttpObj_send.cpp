#include "HttpObj.hpp"

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
/***  								SEND									***/
///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
/**	Try to send from correct source, update _bytes_written
* @return HttpBodyStatus after the send (SENDING_XXX | CLOSED)---*/
HttpObj::HttpBodyStatus	HttpObj::send(char *buff, size_t sizeofbuff, int fd) {
	ssize_t bytesSent;

// send from correct source
	if (_status == HttpObj::SENDING_HEAD)
		bytesSent = sendBufferString(buff, sizeofbuff, fd, _head);
	else if (_status == HttpObj::SENDING_BODY)
		bytesSent = sendBufferString(buff, sizeofbuff, fd, _body);
	else if (_status == HttpObj::SENDING_BODY_FD) {

		if (!_leftovers.empty())
			bytesSent = sendBufferString(buff, sizeofbuff, fd, _leftovers);
		else
			bytesSent = sendBufferFile(buff, sizeofbuff, fd, _tmp_file._fd);
	} else
		return (_status = whatToSend());

// handle errors
	if (bytesSent == 0) {
		LOG_INFO(printFd(fd) << "→ " RED "Connection closed (FIN received)" RESET);
		return (_status = CLOSED);
	}
	if (bytesSent < 0)
		return _status;

// update post sent
	if (_status != SENDING_HEAD)
		_bytes_written += bytesSent;

	if (_bytes_written >= static_cast<size_t>(_bytes_total))
		return (_status = FINISHED);

	return (_status = whatToSend());
}

