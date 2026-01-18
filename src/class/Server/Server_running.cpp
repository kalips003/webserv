#include "Server.hpp"

#include "defines.hpp"
///////////////////////////////////////////////////////////////////////////////]
void    Server::run_better( void ) {

	char buffer[BUFFER_SIZE];

	while (true) {

		accept_clients();
		
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

			if (_events[i].data.fd == _socket_fd)
				accept_clients(); // new connection

			if (_events[i].events & EPOLLERR || _events[i].events & EPOLLRDHUP) {
				oss msg; msg << "[#" C_431 << _events[i].data.fd << RESET "] " RED "connection closed (FIN received)" RESET;
				printLog(DEBUG, msg.str(), 1);
				pop_connec(_clients.find(_events[i].data.fd));
			}

			if (_events[i].events & EPOLLIN) {
				if (!_clients[_events[i].data.fd].ft_update(buffer, sizeof(buffer)))
					pop_connec(_clients.find(_events[i].data.fd));

			}

			if (_events[i].events & EPOLLOUT) {
				if (!_clients[_events[i].data.fd].ft_update(buffer, sizeof(buffer)))
					pop_connec(_clients.find(_events[i].data.fd));

			}
		}
	}
}
