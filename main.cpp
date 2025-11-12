#include "webserv.hpp"

#include "Server.hpp"
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
int main(int ac, char** av)
{
    if (ac != 2) {
	    std::cerr << C_430 "require one .conf file as argument" RESET << std::endl;
        return 0;
    }
    
    Server  server(av[1]);
    if (!server.getStatus())
        return 0;

    server.run_better();

	return 0;
}


///////////////////////////////////////////////////////////////////////////////]
//              BLOCKING SERVER FOR TEST
///////////////////////////////////////////////////////////////////////////////]
// #include <stdio.h> // perror
// #include <iostream>
// #include <cstring>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// int main() {
//     int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

//     struct sockaddr_in addr;
//     std::memset(&addr, 0, sizeof(addr));
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8080);
//     addr.sin_addr.s_addr = INADDR_ANY;

//     bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
//     listen(listen_fd, 5);

//     std::cout << "Server listening on port 8080\n";

//     int client_fd = accept(listen_fd, NULL, NULL);
//     if (client_fd == -1) {
//         perror("accept");
//         close(listen_fd);
//         return 1;
//     }

//     char buffer[1024];
//     while (true) {
//         std::string s;
//         ssize_t received = recv(client_fd, buffer, sizeof(buffer)-1, 0);
//         if (received <= 0) break; // client closed or error

//         buffer[received] = '\0';
//         s += buffer;
//         std::cout << "Client: " << buffer << std::endl;

// 		// echo back to client
// 		send(client_fd, buffer, received, 0);
//     }

//     std::cout << "Client disconnected, shutting down.\n";
//     close(client_fd);
//     close(listen_fd);
//     return 0;
// }
