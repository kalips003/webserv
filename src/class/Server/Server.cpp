#include "Server.hpp"

#include <fcntl.h>
#include <iostream>
#include <cerrno>
#include "Tools1.hpp"
#include "Tools2.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** Remove the given Client from the list
 *
 * @param it   std::map<int, connection> iterator to pop
 * @return     next client in the list		---*/
c_it	Server::pop_connec(c_it it) {

	c_it next = it;
	next++;
	Connection& client = it->second;

	std::cout << RED "closing: " RESET << client.getClientFd() << std::endl;

	if (client.findRequestHeader("connection") == "keep-alive") {
		std::cout << BLUE "we wnat to keep this connec alive" RESET << std::endl;
		client.resetAnswer();
		client.resetRequest();
		client.setStatus(FIRST);
	}
	else {
		it->second.closeFd();
		_clients.erase(it);
	}
	std::cout << RED "how many clients?: " RESET << _clients.size() << std::endl;
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
void	Server::accept_client() {

	struct sockaddr_in	client_addr;
	socklen_t			addr_len = sizeof(client_addr); 

// std::cerr << C_115 "waiting accept" RESET << std::endl;
	int client_fd = accept(_socket_fd, (struct sockaddr*)&client_addr, &addr_len);
	if (client_fd < 0) {
		return ;
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			std::cerr << ERR7 "???\n"; // fcntl()'s fault, no data to read yet
		else 
			printErr(ERR8 "accept() failed");
		return ;
	}

	bool set = set_flags(client_fd, O_NONBLOCK);
	if (!set)
		return ;

	// _clients.insert(std::pair<int, Connection>(client_fd, Connection(client_fd, client_addr, addr_len)));
	

        // Connection    request(client_fd, client_addr, addr_len);
    _clients.insert(std::pair<int, Connection>(client_fd, Connection(client_fd, client_addr, addr_len)));
        // _clients[client_fd] = request;
	// _clients[client_fd] = Connection(client_fd, client_addr, addr_len, _settings);

	std::cout << C_115 "-----------------------------------------]\n";
	std::cout << "New client Accepted: " RESET << _clients[client_fd] << std::endl;
	std::cout << C_115 "-----------------------------------------]" << std::endl;
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
