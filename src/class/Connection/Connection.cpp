#include "Connection.hpp"

#include <unistd.h>
#include <netinet/in.h>
#include <iostream>

#include "HttpStatusCode.hpp"
#include "Task.hpp"
#include "Tools1.hpp"
#include "Tools2.hpp"

///////////////////////////////////////////////////////////////////////////////]
Connection::~Connection() {
	if (_body_task) {
		delete _body_task;
		_body_task = NULL;
	}
}

void	Connection::closeFd() {
	if (_client_fd >= 0) close(_client_fd);
	_client_fd = -1;
}

#include <sys/epoll.h>
///////////////////////////////////////////////////////////////////////////////]
/**	Use internal _status do decide what to do with the given buffer */
bool	Connection::ft_update(char *buff, size_t sizeofbuff) {

	if (_status <= READING_BODY) {
		oss msg; msg << "[#" C_431 << _client_fd <<  RESET "] - READING - " RESET;
		if (_status != FIRST)
			printLog(DEBUG, msg.str(), 1);
		_status = ft_read(buff, sizeofbuff);
		if (_status == SENDING)
			epollChangeFlags(_epoll_fd, _client_fd, EPOLLOUT, EPOLL_CTL_MOD);
	}

	if (_status == DOING) {
		oss msg; msg << "[#" C_431 << _client_fd <<  RESET "] - DOING - " RESET;
		printLog(DEBUG, msg.str(), 1);
		_status = ft_doing();
		if (_status == SENDING)
			epollChangeFlags(_epoll_fd, _client_fd, EPOLLOUT, EPOLL_CTL_MOD);
	}

	else if (_status == SENDING) {
		oss msg; msg << "[#" C_431 << _client_fd <<  RESET "] - SENDING - " RESET;
		printLog(DEBUG, msg.str(), 1);
		_status = ft_send(buff, sizeofbuff);
	}

	if (_status == CLOSED) {
		oss msg; msg << "[#" C_431 << _client_fd <<  RESET "] - CLOSING - " RESET;
		printLog(DEBUG, msg.str(), 1);
		return false;
	}

	// if (DEBUG_MODE == true) {
	// 	// std::cout << DEBUG "[#" << _client_fd << "]" RED " - END LOOP -" RESET << std::endl;
	// 	sleep(1);
	// }

	return true;
}

//?????????????????????????????????????????????????????????????????????????????]
enum ConnectionStatus	Connection::ft_doing( void ) {

	_body_task = Task::createTask(_request.getMethod(), *this);
	if (!_body_task) {
		printLog(ERROR, RED "_body task NULL" RESET, 1);
		return SENDING; // error
	}

	int r = _body_task->ft_do();

	if (r)
		return _answer.create_error(r);
	else
		_answer.http_answer_ini();

	return SENDING;
}

///////////////////////////////////////////////////////////////////////////////]
/**	Try a send to client through given buffer */
enum ConnectionStatus Connection::ft_send(char *buff, size_t sizeofbuff) {

	return _answer.sending(buff, sizeofbuff, _client_fd);
}

#include <arpa/inet.h>
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const Connection& c) {

    // os << C_431 " - Client fd: " RESET << c.getClientFd() << "; ";
    // os << C_152 "STATUS: " RESET << c.getStatus() << std::endl;

	struct sockaddr_in  client_addr = c.getClientAddr();

    os << C_250 << inet_ntoa(client_addr.sin_addr) << RESET ":" << C_502 << ntohs(client_addr.sin_port) << RESET << std::endl;
    // os << C_434 "addr: " RESET << client_addr.sin_addr.s_addr << std::endl;
    // os << C_434 "sin_family: " RESET << client_addr.sin_family << std::endl;
    // os << C_525 "sin_port: " RESET << ntohs(client_addr.sin_port) << std::endl;
    return os;
}


