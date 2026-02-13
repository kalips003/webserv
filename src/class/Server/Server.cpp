#include "Server.hpp"
#include "Log.hpp"

#include <fcntl.h>
#include <iostream>
#include <cerrno>
#include "Tools2.hpp"
#include "Tools1.hpp"
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////]
/** Remove the given Client from the list
 *
 * @param it   std::map<int, connection> iterator to pop
 * @return     next client in the list		---*/
c_it	Server::pop_connec(c_it it, bool timeout) {

	Connection& client = it->second;
	
	if (!timeout && client.getStatus() != Connection::CLOSED && client.findRequestHeader("Connection") == "keep-alive") {
		LOG_INFO(printFd(client.getClientFd()) << C_532 "closing:" RESET " Connection kept-alive.");
		client.resetConnection();
		++it;
	}
	else {
		if (!timeout)
			LOG_INFO(printFd(client.getClientFd()) << C_532 "closing:" RESET " Connection deleted.")
		else
			LOG_INFO(printFd(client.getClientFd()) << C_532 "timeout:" RESET " Connection deleted.")

		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client.getClientFd(), NULL); // <<<<< ??? may fail, and then?
		client.closeFd();

		c_it next = ++it;
		_clients.erase(--it);
		it = next;
	}

	LOG_INFO("Remaining: " C_431 << _clients.size() << RESET " clients.");
	return it;
}

///////////////////////////////////////////////////////////////////////////////]
// #include <fcntl.h>
// #include <iostream>
// #include <cerrno>
// #include "Tools1.hpp"
// #include "Tools2.hpp"

///////////////////////////////////////////////////////////////////////////////]
bool	Server::accept_new_clients(void* ptr, char *buff, size_t sizeofbuff) {

	for (std::map<int, server_listen>::iterator it = _sockets.begin(); it != _sockets.end(); ++it) {
		if (&it->second == ptr) {
			accept_clients(buff, sizeofbuff, static_cast<Server::server_listen*>(ptr));
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////]
/** Try to accept a new client
 *
 * if successful, add it to _clients		---*/
void	Server::accept_clients(char *buff, size_t sizeofbuff, Server::server_listen* this_domain) {

	while (1) {
		Server::ConnectionAcceptResult rtrn = accept_one_client(buff, sizeofbuff, this_domain);

		if (rtrn == ACCEPT_EMPTY)
			break;
		else if (rtrn == ACCEPT_FATAL) {
			LOG_ERROR(RED "Server Rebooted by security. accept()" RESET);
			reboot();
			break;
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
Server::ConnectionAcceptResult	Server::accept_one_client(char *buff, size_t sizeofbuff, Server::server_listen* this_domain) {

	struct sockaddr_in	client_addr;
	socklen_t			addr_len = sizeof(client_addr); 

	int client_fd = accept(this_domain->_socket_fd, (struct sockaddr*)&client_addr, &addr_len);
	if (client_fd < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ACCEPT_EMPTY;
		LOG_ERROR_SYS("accept_one_client(): accept()");
		if (errno == EINTR || errno == ECONNABORTED)
			return ACCEPT_RETRY;
		else
			return ACCEPT_FATAL;
	}

	bool set = set_flags(client_fd, O_NONBLOCK);
	if (!set)
		return ACCEPT_RETRY;

	_clients.insert(std::pair<int, Connection>(client_fd, Connection(client_fd, _epoll_fd, client_addr, addr_len, buff, sizeofbuff, &this_domain->_settings, _cookies)));

	std::map<int, Connection>::iterator conn_it = _clients.find(client_fd);
	if (!epollChangeFlags(_epoll_fd, client_fd, &conn_it->second, EPOLLIN, EPOLL_CTL_ADD))
		return ACCEPT_RETRY;

	LOG_INFO("New client Accepted: " << printFd(client_fd) << conn_it->second);
	return ACCEPT_OK;
}

#include <cstdlib>
//-----------------------------------------------------------------------------]
/**	Reset the server, and timeout 10s */
void	Server::reboot() {

// destructor:
	for (map_clients::iterator it = _clients.begin(); it != _clients.end(); )
		it = pop_connec(it, false);
	if (_epoll_fd >= 0) { close(_epoll_fd); _epoll_fd = -1; }
	for (std::map<int, server_listen>::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
		if (it->second._socket_fd >= 0) { close(it->second._socket_fd); it->second._socket_fd = -1; }

	for (int i = 10; i > 0; --i) {
		LOG_ERROR(RED "Reboot in ... " RESET << i);
		sleep(1);
	}
	if (!create_all_listening_socket() || !create_epoll()) {
		LOG_ERROR(RED "FATAL ERROR" RESET);
		exit(1);
	}
	LOG_ERROR(GREEN "REBOOT SUCCESSFUL" RESET);
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream&	operator<<(std::ostream& os, Server& s) {

	os << C_542 "---------------------------------------------\n" RESET;
	os << C_542 "\t- HELLO THIS IS SERVER -\n\n" RESET;
	os << C_542 "---------------------------------------------\n" RESET;
	for (map_clients::const_iterator it = s._clients.begin(); it != s._clients.end(); ++it)
		os << it->second;

	return os;
}
