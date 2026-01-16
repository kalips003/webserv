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

	std::cout << INFO "[#" C_431 << client.getClientFd() <<  RESET "] closing: ";

	if (client.findRequestHeader("connection") == "keep-alive") {
		std::cout << "Connection kept-alive." << std::endl;
		client.resetAnswer();
		client.resetRequest();
		client.setStatus(FIRST);
	}
	else {
		std::cout << "Connection deleted." << std::endl;
		it->second.closeFd();
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

	if (DEBUG_MODE == true) std::cout << INFO "New client Accepted: [#" C_431 << client_fd <<  RESET "] " << _clients[client_fd];

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
