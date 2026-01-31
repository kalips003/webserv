#include "Server.hpp"

#include "defines.hpp"
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
///////////////////////////////////////////////////////////////////////////////]
void    Server::run( void ) {

	char buffer[BUFFER_SIZE];

	while (true) {

		int nfds = epoll_wait(_epoll_fd, _events, MAX_EVENTS, -1); // timeout??
		if (nfds == -1) {
			if (errno == EINTR)
				continue;
			else {
				printErr("epoll_wait() FATAL ERROR");
				break;
			}
		}
		for (int i = 0; i < nfds; ++i) {
			if (_events[i].data.ptr == this) {
				accept_clients(buffer, sizeof(buffer)); // new connection
				continue;
			}

			if (_events[i].events & EPOLLERR || _events[i].events & EPOLLRDHUP) {
				LOG_INFO(static_cast<Connection*>(_events[i].data.ptr)->getClientFd() << RED "connection closed (FIN received)" RESET);
				pop_connec(_clients.find(static_cast<Connection*>(_events[i].data.ptr)->getClientFd()));
			}

			if (_events[i].events & EPOLLIN) {
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
