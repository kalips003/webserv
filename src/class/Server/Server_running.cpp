#include "Server.hpp"

#include "defines.hpp"
///////////////////////////////////////////////////////////////////////////////]
void    Server::run_better( void ) {

	char buffer[4096];

	while (true) {

		accept_client();
		
		for (c_it it = _clients.begin(); it != _clients.end(); ) {

			if (!it->second.ft_update(buffer, sizeof(buffer)))
				it = pop_connec(it);
			else
				++it;
		}
	}
}

#include <string.h>
#include <unistd.h>
#include <iostream>
#include "Tools1.hpp"
///////////////////////////////////////////////////////////////////////////////]
void    Server::run_simple( void ) {

//     while (true) {
// std::cout << C_431 "waiting on accept()" RESET << std::endl;

//         struct sockaddr_in  client_addr;
//         socklen_t           addr_len;
//         int client_fd = accept(_socket_fd, (struct sockaddr*)&client_addr, &addr_len);

//         if (client_fd < 0) {
//             printErr(RED "accept() failed" RESET);
//             continue;
//         }
// std::cout << C_431 "\taccepted..." RESET << std::endl;


//         // Connection    request(client_fd, client_addr, addr_len);
//         // _clients[client_fd] = request;
// std::cout << C_241 "\trequest created..." RESET << std::endl;

// std::cout << C_241 "\treading..." RESET << std::endl;
//         // int r = request.recv_all_buffer();
// // std::cout << C_241 "\tfinished, status: " RESET << r << std::endl;

// std::cout << C_253 "\tsending answer..." RESET << std::endl;

//         const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
//         send(request.getClientFd(), response, strlen(response), 0);


//         close(request.getClientFd());
// std::cout << C_253 "\tfinished..." RESET << std::endl;
//     }
}