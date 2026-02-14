#include "Connection.hpp"

#include "Tools1.hpp"
#include "Tools2.hpp"
#include "Method.hpp"

#include <sys/epoll.h>

///////////////////////////////////////////////////////////////////////////////]
/**	Use internal _status do decide what to do with the given buffer */
bool	Connection::ft_update(char *buff, size_t sizeofbuff) {
LOG_LOG("ft_update(): " << printFd(_data._client_fd))
	updateTimeout();

	if (_status == READING) {
		if (_request.getStatus() != HttpObj::READING_FIRST)
			LOG_LOG(printFd(_data._client_fd) << "--- READING --- ");

		_status = ft_read(buff, sizeofbuff);
		if (_status == SENDING)
			epollChangeFlags(_data._epoll_fd, _data._client_fd, this, EPOLLOUT, EPOLL_CTL_MOD);
	}

	if (_status == DOING || _status == DOING_CGI) {
		LOG_LOG(printFd(_data._client_fd) << "--- DOING --- " << (_status == DOING_CGI ? "CGI ---" : ""));
		
		_status = ft_doing();
		if (_status == SENDING) {
			LOG_INFO(printFd(_data._client_fd) << "→ " << _answer.getFirst())
			epollChangeFlags(_data._epoll_fd, _data._client_fd, this, EPOLLOUT, EPOLL_CTL_MOD);
		}
	}

	if (_status == SENDING) {
		LOG_LOG(printFd(_data._client_fd) << "--- SENDING --- ");
		_status = ft_send(buff, sizeofbuff);
	}

	if (_status == CLOSED || _status == FINISHED) {
		LOG_LOG(printFd(_data._client_fd) << "--- CLOSING --- ");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////]
/** @brief Read from provided buffer once
//
// _request update internally, return the status after recv()
//
// if error from _request, fills _answer for the error
* @return CONNECTION_STATUS		---*/
Connection::ConnectionStatus Connection::ft_read(char *buff, size_t sizeofbuff) {
// LOG_LOG("ft_read()")

	int rtrn = _request.receive(buff, sizeofbuff, _data._client_fd, recv0);

	if (rtrn >= 100) {
		_answer.createError(rtrn, _request.getMethod());
		return SENDING;
	}

	if (rtrn == HttpObj::CLOSED) {
		LOG_DEBUG("ft_read(): CLOSED after receive()")
		return CLOSED;
	}
	else if (rtrn < HttpObj::DOING)
		return READING;
	else if (rtrn == HttpObj::DOING)
		return DOING;

	return SENDING;
}


#include "Tools2.hpp"
///////////////////////////////////////////////////////////////////////////////]
/**	execute the Method class, depending on status DOING or DOING_CGI
// @return ConnectionStatus */
Connection::ConnectionStatus	Connection::ft_doing( void ) {
// LOG_LOG("ft_doing()")
	int rtrn;

// if first time, create Method
	if (!_body_task)
		_body_task = Method::createTask(_request.getMethod(), _data);
	if (!_body_task) {
		LOG_ERROR(RED "_body task NULL" RESET);
		_answer.createError(500, _request.getMethod());
		return SENDING;
	}

// exec Method
	if (_status == DOING_CGI) {
		rtrn = _body_task->exec_cgi();
		if (rtrn != DOING_CGI)
			epollChangeFlags(_data._epoll_fd, _data._client_fd, this, EPOLLOUT, EPOLL_CTL_ADD);
	}
	else {
		_body_task->printHello();
		rtrn = _body_task->normal_doing();
	}

// check return
	if (rtrn >= 100) {
		_answer.createError(rtrn, _request.getMethod());
		return SENDING;
	}
	if (rtrn == SENDING) {
		std::string* connection = _request.find_in_headers("Connection");
		if (connection)
			_answer.addToHeaders("connection", *connection);
		_answer.initializationBeforeSend(_request.getMethod());
	}

	return static_cast<Connection::ConnectionStatus>(rtrn);
}

///////////////////////////////////////////////////////////////////////////////]
/**	Try a send to client through given buffer */
Connection::ConnectionStatus 	Connection::ft_send(char *buff, size_t sizeofbuff) {

	HttpObj::HttpBodyStatus r = _answer.send(buff, sizeofbuff, _data._client_fd);
	if (r == HttpObj::CLOSED)
		return Connection::CLOSED;
	else if (r == HttpObj::FINISHED)
		return Connection::FINISHED;
	return Connection::SENDING;
}
