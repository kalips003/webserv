#include "Server.hpp"

#include <fcntl.h>
#include <iostream>
#include <cerrno>
#include "Tools1.hpp"
#include "Tools2.hpp"
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////]
/** Remove the given Client from the list
 *
 * @param it   std::map<int, connection> iterator to pop
 * @return     next client in the list		---*/
c_it	Server::pop_connec(c_it it) {

	c_it next = it;
	next++;
	Connection& client = it->second;

	std::cout << INFO "[#" C_431 << client.getClientFd() <<  RESET "] closing: ";

	if (client.findRequestHeader("connection") == "keep-alive" && client.findAnswertHeader("Connection") != "close") {
		std::cout << "Connection kept-alive." << std::endl;
		client.resetConnection();
	}
	else {
		std::cout << "Connection deleted." << std::endl;
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client.getClientFd(), NULL); // <<<<< ??? may fail, and then?
		client.closeFd();
		_clients.erase(it);
	}
	if (DEBUG_MODE == true) std::cout << INFO "Remaining: " C_431 << _clients.size() << RESET " clients." << std::endl;
	return next;
}

///////////////////////////////////////////////////////////////////////////////]
// #include <fcntl.h>
// #include <iostream>
// #include <cerrno>
// #include "Tools1.hpp"
// #include "Tools2.hpp"
///////////////////////////////////////////////////////////////////////////////]
/** Try to accept a new client
 *
 * if successful, add it to _clients		---*/
void	Server::accept_clients(char *buff, size_t sizeofbuff) {

	while (1) {
		Server::ConnectionAcceptResult rtrn = accept_one_client(buff, sizeofbuff);

		if (rtrn == ACCEPT_EMPTY)
			break;
		else if (rtrn == ACCEPT_FATAL) {
			printErr(RED "Server Rebooted by security. accept()" RESET);
			reboot();
		}
	}
}


#include "Tools2.hpp"
///////////////////////////////////////////////////////////////////////////////]
    // ACCEPT_OK = 1,        // one client accepted
    // ACCEPT_EMPTY = 0,     // no more clients (EAGAIN)
    // ACCEPT_RETRY = -1,    // EINTR / ECONNABORTED
    // ACCEPT_FATAL = -2   
///////////////////////////////////////////////////////////////////////////////]
Server::ConnectionAcceptResult	Server::accept_one_client(char *buff, size_t sizeofbuff) {

	struct sockaddr_in	client_addr;
	socklen_t			addr_len = sizeof(client_addr); 

	int client_fd = accept(_socket_fd, (struct sockaddr*)&client_addr, &addr_len);
	if (client_fd < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ACCEPT_EMPTY;
		printErr("accept()");
		if (errno == EINTR || errno == ECONNABORTED)
			return ACCEPT_RETRY;
		else
			return ACCEPT_FATAL;
	}

	bool set = set_flags(client_fd, O_NONBLOCK);
	if (!set)
		return ACCEPT_RETRY;

	_clients.insert(std::pair<int, Connection>(client_fd, Connection(client_fd, _epoll_fd, client_addr, addr_len, buff, sizeofbuff)));

	if (!epollChangeFlags(_epoll_fd, client_fd, &_clients[client_fd], EPOLLIN, EPOLL_CTL_ADD))
		return ACCEPT_RETRY;

	oss msg; msg << "New client Accepted: [#" C_431 << client_fd <<  RESET "] " << _clients[client_fd];
	printLog(INFO, msg.str(), 1);
	return ACCEPT_OK;
}

//-----------------------------------------------------------------------------]
/**	Reset the server, and timeout 10s */
bool	Server::reset() {

// destructor:
	for (map_clients::iterator it = _clients.begin(); it != _clients.end(); )
		it = pop_connec(it);
	if (_epoll_fd >= 0) { close(_epoll_fd); _epoll_fd = -1; }
	if (_socket_fd >= 0) { close(_socket_fd); _socket_fd = -1; }

	_server_status = create_listening_socket();
	if (!_server_status)
		return false;

	_server_status = create_epoll();
	if (!_server_status)
		return false;
	
	return true;
}

#include <cstdlib>
//-----------------------------------------------------------------------------]
/**	Reset the server, and timeout 10s */
void	Server::reboot() {

// destructor:
	for (map_clients::iterator it = _clients.begin(); it != _clients.end(); )
		it = pop_connec(it);
	if (_epoll_fd >= 0) { close(_epoll_fd); _epoll_fd = -1; }
	if (_socket_fd >= 0) { close(_socket_fd); _socket_fd = -1; }

	oss msg;
	for (int i = 10; i > 0; --i) {
		msg.str(""); msg.clear(); msg << RED "Reboot in ... " RESET << i; printLog(ERROR, msg.str(), 1);
		sleep(1);
	}
	if (!create_listening_socket() || !create_epoll()) {
		msg.str(""); msg.clear(); msg << RED "FATAL ERROR" RESET; printLog(ERROR, msg.str(), 1);
		exit(1);
	}
	msg.str(""); msg.clear(); msg << GREEN "REBOOT SUCCESSFUL" RESET; printLog(ERROR, msg.str(), 1);
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream&	operator<<(std::ostream& os, Server& s) {

	os << C_542 "---------------------------------------------\n" RESET;
	os << C_542 "\t- HELLO THIS IS SERVER -\n\n" RESET;
	for (map_clients::const_iterator it = s.getClients().begin(); it != s.getClients().end(); ++it)
		os << it->second;

	return os;
}
