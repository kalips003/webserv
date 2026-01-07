#include "Connection.hpp"

#include <unistd.h>
#include <netinet/in.h>
#include <iostream>

#include "HttpStatusCode.hpp"
#include "Task.hpp"
#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
Connection::~Connection() {

	if (_client_fd >= 0) close(_client_fd);
}

///////////////////////////////////////////////////////////////////////////////]
/**	Use internal _status do decide what to do with the given buffer */
bool	Connection::ft_update(char *buff, size_t sizeofbuff) {

	if (_status <= READING_BODY) {
		std::cout << C_515 "-----------------------------------------]\n";
		std::cout << *this << C_515 "\n\tstatus: " C_411 "- READING -\n";
		std::cout << C_515 "-----------------------------------------]" << std::endl;
		ft_read(buff, sizeofbuff);
		std::cout << C_515 "-----------------------------------------]" << std::endl;
	}

	if (_status == DOING) {
		std::cout << C_512 "-----------------------------------------]\n";
		std::cout << *this << C_512 "\n\tstatus: " C_411 "- DOING -\n";
		ft_doing();
		std::cout << C_512 "-----------------------------------------]" << std::endl;
	}
	
	if (_status == SENDING) {
		std::cout << C_431 "-----------------------------------------]\n";
		std::cout << *this << C_431 "\n\tstatus: " C_411 "- SENDING -\n";
		ft_send(buff, sizeofbuff);
		std::cout << C_431 "-----------------------------------------]" << std::endl;
	}

	if (_status == CLOSED) {
		std::cout << C_330 "-----------------------------------------]\n";
		std::cout << *this << C_330 "\n\tstatus: " C_411 "- CLOSED -\n";
		std::cout << C_330 "\n-----------------------------------------]" << std::endl;
		return false;
	}
	return true;
}


//?????????????????????????????????????????????????????????????????????????????]
enum ConnectionStatus	Connection::ft_doing( void ) {
	if (!_body_task)
		return SENDING; // error
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


