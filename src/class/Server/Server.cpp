#include "Server.hpp"

#include "Tools1.hpp"

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

#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////]
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
