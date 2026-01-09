#include "Connection.hpp"

#include <unistd.h>
#include <netinet/in.h>
#include <iostream>

#include "HttpStatusCode.hpp"
#include "Task.hpp"
#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
Connection::~Connection() {
	// if (_client_fd >= 0) close(_client_fd);
}

void	Connection::closeFd() {
	if (_client_fd >= 0) close(_client_fd);
	_client_fd = -1;
}

///////////////////////////////////////////////////////////////////////////////]
/**	Use internal _status do decide what to do with the given buffer */
bool	Connection::ft_update(char *buff, size_t sizeofbuff) {
std::cout << *this;

	if (_status <= READING_BODY) {
		std::cout << C_515 "- " C_411 "READING" C_515 " -------------------------------]\n";
		ft_read(buff, sizeofbuff);
	}

	if (_status == DOING) {
		std::cout << C_512 "- " C_411 "DOING" C_512 " ---------------------------------]\n";
		_status = ft_doing();
	}
	
	if (_status == SENDING) {
		std::cout << C_431 "- " C_123 "SENDING" C_431 " -------------------------------]\n";
		_status = ft_send(buff, sizeofbuff);
	}

	if (_status == CLOSED) {
		std::cout << C_330 "- " C_512 "CLOSED" C_330 " --------------------------------]\n";
		return false;
	}
	std::cout << RED "- END LOOP --------------------------------]\n";

	sleep(1);
	return true;
}


//?????????????????????????????????????????????????????????????????????????????]
enum ConnectionStatus	Connection::ft_doing( void ) {

	_body_task = Task::createTask(_request.getMethod(), *this);
	if (!_body_task)
		return SENDING; // error
// //
	{
		std::cerr << RED "no bodytask" << std::endl;
		_answer.create_error(404);
		return SENDING;
	}
// //
	// std::cout << "heelelelelelelelellooo\n";
	int r = _body_task->ft_do();

	if (r)
		_answer.create_error(r);
	else
		_answer.http_answer_ini();
	return SENDING;
}

///////////////////////////////////////////////////////////////////////////////]
/**	Try a send to client through given buffer */
enum ConnectionStatus Connection::ft_send(char *buff, size_t sizeofbuff) {

	return _answer.sending(buff, sizeofbuff, _client_fd);
}

// #include <netinet/in.h>
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const Connection& c) {

    os << C_152 "Client n." RESET << c.getClientFd() << std::endl;
    os << C_152 "STATUS: " RESET << c.getStatus() << std::endl;

	struct sockaddr_in  client_addr = c.getClientAddr();

    os << C_434 "addr: " RESET << client_addr.sin_addr.s_addr << std::endl;
    os << C_434 "sin_family: " RESET << client_addr.sin_family << std::endl;
    os << C_434 "sin_port: " RESET << client_addr.sin_port << std::endl;
    return os;
}


