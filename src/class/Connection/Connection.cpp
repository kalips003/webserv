#include "Connection.hpp"
#include "Log.hpp"

#include <unistd.h>
#include <netinet/in.h>
#include <iostream>

#include "HttpStatusCode.hpp"
#include "HttpObj.hpp"
#include "Method.hpp"
#include "Tools1.hpp"
#include "Tools2.hpp"

///////////////////////////////////////////////////////////////////////////////]
Connection::~Connection() {
LOG_DEBUG("Connection destructor called [" << _data._client_fd << "]");
	if (_body_task) {
		delete _body_task;
		_body_task = NULL;
	}
}

void	Connection::closeFd() {
	if (_data._client_fd >= 0) close(_data._client_fd);
	_data._client_fd = -1;
}

#include <sys/epoll.h>
///////////////////////////////////////////////////////////////////////////////]
/**	Use internal _status do decide what to do with the given buffer */
bool	Connection::ft_update(char *buff, size_t sizeofbuff) {

	if (_status == READING) {
		if (_request.getStatus() != HttpObj::READING_FIRST)
			LOG_DEBUG(printFd(_data._client_fd) << "--- READING --- ");

		_status = ft_read(buff, sizeofbuff);
		if (_status == SENDING)
			epollChangeFlags(_data._epoll_fd, _data._client_fd, this, EPOLLOUT, EPOLL_CTL_MOD);
	}

	if (_status == DOING || _status == DOING_CGI) {
		LOG_DEBUG(printFd(_data._client_fd) << "--- DOING --- ");
		
		_status = ft_doing();
		if (_status == SENDING)
			epollChangeFlags(_data._epoll_fd, _data._client_fd, this, EPOLLOUT, EPOLL_CTL_MOD);
	}

	if (_status == SENDING) {
		LOG_DEBUG(printFd(_data._client_fd) << "--- SENDING --- ");
		_status = ft_send(buff, sizeofbuff);
	}

	if (_status == CLOSED) {
		LOG_DEBUG(printFd(_data._client_fd) << "--- CLOSING --- ");
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

	int rtrn = _request.receive(buff, sizeofbuff, _data._client_fd, recv0);

	if (rtrn >= 100) {
		_answer.createError(rtrn);
		return SENDING;
	}
	HttpObj::HttpBodyStatus r = static_cast<HttpObj::HttpBodyStatus>(rtrn);

	if (r == HttpObj::CLOSED) {
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

// if first time, create Method
	if (!_body_task)
		_body_task = Method::createTask(_request.getMethod(), _data);
	if (!_body_task) {
		LOG_ERROR(RED "_body task NULL" RESET);
		_answer.createError(500);
		return SENDING;
	}

// exec Method
	_body_task->printHello();
	int rtrn;
	if (_status == DOING_CGI) {
		rtrn = _body_task->exec_cgi();
		if (rtrn != DOING_CGI)
			epollChangeFlags(_data._epoll_fd, _data._client_fd, this, EPOLLOUT, EPOLL_CTL_ADD);
	}
	else
		rtrn = _body_task->normal_doing();

// check return
	if (rtrn >= 100) {
		_answer.createError(rtrn);
		return SENDING;
	}
	if (rtrn == SENDING)
		_answer.initializationBeforeSend();
	return static_cast<Connection::ConnectionStatus>(rtrn);
}

#include "Method.hpp"
///////////////////////////////////////////////////////////////////////////////]
/**	Try a send to client through given buffer */
Connection::ConnectionStatus 	Connection::ft_send(char *buff, size_t sizeofbuff) {

	HttpObj::HttpBodyStatus r = _answer.send(buff, sizeofbuff, _data._client_fd);
	if (r == HttpObj::CLOSED)
		return Connection::CLOSED;
	return Connection::SENDING;
}

///////////////////////////////////////////////////////////////////////////////]
/***  								GETTERS									***/
///////////////////////////////////////////////////////////////////////////////]

//-----------------------------------------------------------------------------]
/** find the given setting in the _headers
*	@return the string value of the setting
*
* if setting not found, return empty ""	---*/
std::string		Connection::findRequestHeader(const std::string& header) {
	const std::string* rtrn = _request.find_in_headers(header);
	return rtrn ? *rtrn : "";
}
//-----------------------------------------------------------------------------]
/** find the given setting in the _headers
*	@return the string value of the setting
*
* if setting not found, return empty ""	---*/
std::string		Connection::findAnswertHeader(const std::string& header) {
	const std::string* rtrn = _answer.find_in_headers(header);
	return rtrn ? *rtrn : "";
}

///////////////////////////////////////////////////////////////////////////////]
/***  								SETTERS									***/
///////////////////////////////////////////////////////////////////////////////]

//-----------------------------------------------------------------------------]
void	 Connection::resetConnection() {
	resetAnswer();
	resetRequest();
	delete _body_task;
	_body_task = NULL;
}

#include <arpa/inet.h>
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const Connection& c) {

	// os << C_431 " - Client fd: " RESET << c.getClientFd() << "; ";
	// os << C_152 "STATUS: " RESET << c.getStatus() << std::endl;

	struct sockaddr_in  client_addr = c.getClientAddr();

	os << C_250 << inet_ntoa(client_addr.sin_addr) << RESET ":" << C_431 << ntohs(client_addr.sin_port) << RESET;
	// os << C_434 "addr: " RESET << client_addr.sin_addr.s_addr << std::endl;
	// os << C_434 "sin_family: " RESET << client_addr.sin_family << std::endl;
	// os << C_525 "sin_port: " RESET << ntohs(client_addr.sin_port) << std::endl;
    return os;
}

///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const Connection::transfer_data& t) {
	os << "transfer_data { "
		<< "client_fd=" << t._client_fd
		<< ", epoll_fd=" << t._epoll_fd
		<< ", this_ptr=" << t._this_ptr
		<< ", buffer=" << static_cast<void*>(t._buffer)
		<< ", sizeofbuff=" << t._sizeofbuff
		<< " }";
	return os;
}

