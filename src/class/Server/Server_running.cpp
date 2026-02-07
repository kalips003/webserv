#include "Server.hpp"

#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <sys/time.h>

#include "Tools1.hpp"

// #define EPOLL_TIMEOUT -1
#define EPOLL_TIMEOUT 1000
///////////////////////////////////////////////////////////////////////////////]
void    Server::run( void ) {

	char buffer[BUFFER_SIZE];

	while (!g_ServerEnd) {

	// cleanup
		for (map_clients::iterator it = _clients.begin(); it != _clients.end(); ) {
			timeval now;
			gettimeofday(&now, NULL);
			if (it->second.checkTimeout(now))
				it = pop_connec(it, true);
			else
				++it;
		}

	// wait events
		int nfds = epoll_wait(_epoll_fd, _events, MAX_EVENTS, EPOLL_TIMEOUT);
		if (nfds == -1) {
			if (errno == EINTR)
				continue;
			else {
				LOG_ERROR("epoll_wait() FATAL ERROR");
				break;
			}
		}

	// execute events
		for (int i = 0; i < nfds && !g_ServerEnd; ++i) {

			if (accept_new_clients(_events[i].data.ptr, buffer, sizeof(buffer))) // new connection
				continue;

			if (_events[i].events & EPOLLERR) {// An error condition happened on the socket
				handle_EPOLLERR(static_cast<Connection*>(_events[i].data.ptr));
				continue;
			}
			if (_events[i].events & EPOLLRDHUP) {// Peer closed the connection (FIN received) or half-closed for reading
				handle_EPOLLRDHUP(static_cast<Connection*>(_events[i].data.ptr));
				continue;
			}
			if (_events[i].events & EPOLLHUP) {// The socket was hung up
				handle_EPOLLHUP(static_cast<Connection*>(_events[i].data.ptr), buffer, sizeof(buffer));
				continue;
			}

			if (_events[i].events & EPOLLIN) {
				if (!static_cast<Connection*>(_events[i].data.ptr)->ft_update(buffer, sizeof(buffer)))
					pop_connec(_clients.find(static_cast<Connection*>(_events[i].data.ptr)->getClientFd()), false);
			}

			if (_events[i].events & EPOLLOUT) {
				if (!static_cast<Connection*>(_events[i].data.ptr)->ft_update(buffer, sizeof(buffer)))
					pop_connec(_clients.find(static_cast<Connection*>(_events[i].data.ptr)->getClientFd()), false);
			}
		}
	}
}

#include <cstring>
///////////////////////////////////////////////////////////////////////////////]
void	Server::handle_EPOLLERR(Connection* client) {
LOG_HERE("in handle_EPOLLERR")
	int err = 0;
	socklen_t len = sizeof(err);
	if (getsockopt(client->getClientFd(), SOL_SOCKET, SO_ERROR, &err, &len) == -1)
		LOG_ERROR_SYS(printFd(client->getClientFd()) << RED "EPOLLERR " RESET "getsockopt(): " << strerror(errno))
	else if (err != 0) {
		LOG_ERROR_SYS(printFd(client->getClientFd()) << RED "EPOLLERR: " RESET << strerror(err))
	}
	LOG_INFO(printFd(client->getClientFd()) << RED "(Server) connection closed due to socket error" RESET)
	client->getRequest().addToHeaders("connection", "closed");
	pop_connec(_clients.find(client->getClientFd()), false);
}
///////////////////////////////////////////////////////////////////////////////]
void	Server::handle_EPOLLRDHUP(Connection* client) {
LOG_HERE("in handle_EPOLLRDHUP")
	LOG_INFO(printFd(client->getClientFd()) << RED "(Server) connection closed by peer (FIN received)" RESET);

	// We can still send remaining data if needed, but for simplicity, we'll close the connection.
	client->getRequest().addToHeaders("Connection", "closed");
	pop_connec(_clients.find(client->getClientFd()), false);
}
///////////////////////////////////////////////////////////////////////////////]
void	Server::handle_EPOLLHUP(Connection* client, char* buffer, size_t sizeofbuff) {
LOG_HERE("in handle_EPOLLHUP")
	if (client->getStatus() == Connection::DOING_CGI) { // the cgi pipe sent EOF
		if (!client->ft_update(buffer, sizeofbuff))
			pop_connec(_clients.find(client->getClientFd()), false);
	}
	else // its the fd of the client that closed
		handle_EPOLLRDHUP(client);
}