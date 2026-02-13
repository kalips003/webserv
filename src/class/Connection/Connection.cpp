#include "Connection.hpp"

#include <sys/time.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

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

//-----------------------------------------------------------------------------]
void	Connection::closeFd() {
	if (_data._client_fd >= 0) close(_data._client_fd);
	_data._client_fd = -1;
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

//-----------------------------------------------------------------------------]
bool	Connection::checkTimeout(const timeval& now) {
	double time_elapsed = (now.tv_sec - _last_active.tv_sec) + (now.tv_usec - _last_active.tv_usec) / 1e6;

	if (_status == Connection::DOING_CGI && time_elapsed >= CONNECTION_TIMEOUT) {

		LOG_WARNING(printFd(_data._client_fd) << "CGI timeout, sending 504 response");
		if (_body_task) {
			delete _body_task;
			_body_task = NULL;
		}
		resetAnswer();
		_answer.createError(504, _request.getMethod());
		epollChangeFlags(_data._epoll_fd, _data._client_fd, this, EPOLLOUT, EPOLL_CTL_MOD);
		updateTimeout();
		_status = Connection::SENDING;
		return false;
	}
	
	return time_elapsed >= CONNECTION_TIMEOUT;
}
///////////////////////////////////////////////////////////////////////////////]
/***  								SETTERS									***/
///////////////////////////////////////////////////////////////////////////////]

//-----------------------------------------------------------------------------]
void	 Connection::resetConnection() {
// LOG_HERE("resetConnection()")

	resetAnswer();
	resetRequest();
	delete _body_task;
	_body_task = NULL;
// 
	_request.addToHeaders("connection", "keep-alive");
// 
	_status = Connection::READING;
	epollChangeFlags(_data._epoll_fd, _data._client_fd, this, EPOLLIN, EPOLL_CTL_MOD);
	updateTimeout();
}


//-----------------------------------------------------------------------------]
void	Connection::updateTimeout() {
	gettimeofday(&_last_active, NULL);
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const Connection& c) {

	struct sockaddr_in  client_addr = c.getClientAddr();
	os << C_250 << inet_ntoa(client_addr.sin_addr) << RESET ":" << C_431 << ntohs(client_addr.sin_port) << RESET;

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

