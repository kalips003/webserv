#include "Log.hpp"
#include "Server.hpp"

#include "defines.hpp"
#include <unistd.h>

bool	g_ServerEnd;
///////////////////////////////////////////////////////////////////////////////]
void    Server::run_better( void ) {

	char buffer[BUFFER_SIZE];

	while (true) {

		accept_clients(buffer, sizeof(buffer));
		
		for (c_it it = _clients.begin(); it != _clients.end(); ) {

			if (!it->second.ft_update(buffer, sizeof(buffer)))
				it = pop_connec(it);
			else
				++it;
		}
	}
}

#include "Tools1.hpp"
#include <cerrno>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////]
void    Server::run( void ) {

	char buffer[BUFFER_SIZE];

	while (!g_ServerEnd) {
		int nfds = epoll_wait(_epoll_fd, _events, MAX_EVENTS, -1); // timeout??
		if (nfds == -1) {
			if (errno == EINTR)
				continue;
			else {
				LOG_ERROR("epoll_wait() FATAL ERROR");
				break;
			}
		}

		for (int i = 0; i < nfds && !g_ServerEnd; ++i) {

			if (_events[i].data.ptr == this) {
				accept_clients(buffer, sizeof(buffer)); // new connection
				continue;
			}

			// if (_events[i].events & EPOLLERR || _events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
			if (_events[i].events & EPOLLERR || _events[i].events & EPOLLRDHUP) {
				LOG_INFO(printFd(static_cast<Connection*>(_events[i].data.ptr)->getClientFd()) << RED "(Server) connection closed (FIN received)" RESET);
				pop_connec(_clients.find(static_cast<Connection*>(_events[i].data.ptr)->getClientFd()));
			}

			if (_events[i].events & (EPOLLIN | EPOLLHUP)) {
				if (!static_cast<Connection*>(_events[i].data.ptr)->ft_update(buffer, sizeof(buffer)))
					pop_connec(_clients.find(static_cast<Connection*>(_events[i].data.ptr)->getClientFd()));
			}

			if (_events[i].events & EPOLLOUT) {
				if (!static_cast<Connection*>(_events[i].data.ptr)->ft_update(buffer, sizeof(buffer)))
					pop_connec(_clients.find(static_cast<Connection*>(_events[i].data.ptr)->getClientFd()));
			}
		}
	}
}

// body: "grant_type=client_credentials&client_id=u-s4t2ud-c226cd35cd1ac08a4c6668deee1c64d7d67a13a766aee672acafd4a1522d483c&client_secret=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",
