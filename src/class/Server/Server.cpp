#include "Server.hpp"

///////////////////////////////////////////////////////////////////////////////]
c_it	Server::pop_connec(c_it it) {

	c_it next = it;
	next++;
	connection& client = it->second;

	std::cout << RED "closing: " RESET << client;

	if (client._request.headers["connection"] == "keep-alive") {
		client._request = http_request(); // clear just
		client._answer = http_answer(); // clear just
		client._buffer.clear();
		client._status = READING_HEADER;
	}
	else
		_clients.erase(it);
	return next;
}


///////////////////////////////////////////////////////////////////////////////]
#include <fcntl.h>
///////////////////////////////////////////////////////////////////////////////]
void	Server::accept_client() {

	struct sockaddr_in	client_addr;
	socklen_t			addr_len = sizeof(client_addr); 

// std::cerr << C_115 "waiting accept" RESET << std::endl;
	int client_fd = accept(_socket_fd, (struct sockaddr*)&client_addr, &addr_len);
	if (client_fd < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			std::cerr << ""; // fcntl()'s fault, no data to read yet
		else 
			printErr(RED "accept() failed" RESET);
		return ;
	}

	bool set = set_flags(client_fd, O_NONBLOCK);
	if (!set)
		return ;

	_clients[client_fd] = connection(client_fd, client_addr, addr_len);

	std::cerr << C_115 "-----------------------------------------]\n";
	std::cerr << "New client Accepted: " RESET << _clients[client_fd] << std::endl;
	std::cerr << C_115 "-----------------------------------------]" << std::endl;
}
