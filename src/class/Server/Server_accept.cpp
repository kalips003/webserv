#include "Server.hpp"

#include <fcntl.h>
#include <cerrno>

#include "Tools1.hpp"
#include "Tools2.hpp"

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
