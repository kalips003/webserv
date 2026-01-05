#include "Server.hpp"

#include "defines.hpp"
///////////////////////////////////////////////////////////////////////////////]
void    Server::run_better( void ) {

    char buffer[4096];

    while (true) {

        accept_client();
    
        for (c_it it = _clients.begin(); it != _clients.end(); ) {

            Connection &connec = it->second;
            
            if (connec.getStatus() <= READING_BODY) {
                std::cerr << C_515 "-----------------------------------------]\n";
                std::cerr << connec << C_515 "\n\tstatus: " RESET << C_411 "- READING -\n";
                connec.ft_read(buffer, sizeof(buffer));
                std::cerr << C_515 "-----------------------------------------]" << std::endl;
            }

            if (connec.getStatus() == DOING) {
                std::cerr << C_512 "-----------------------------------------]\n";
                std::cerr << connec << C_512 "\n\tstatus: " RESET << C_411 "- DOING -\n";
                connec._status = connec.ft_doing();
                std::cerr << C_512 "-----------------------------------------]" << std::endl;
            }
            
            if (connec.getStatus() == SENDING) {
                std::cerr << C_431 "-----------------------------------------]\n";
                std::cerr << connec << C_431 "\n\tstatus: " RESET << C_411 "- SENDING -\n";
                connec._status = connec.ft_send(buffer, sizeof(buffer));
                std::cerr << C_431 "-----------------------------------------]" << std::endl;
            }

            if (connec.getStatus() == CLOSED) {
                std::cerr << C_330 "-----------------------------------------]\n";
                std::cerr << connec << C_330 "\n\tstatus: " RESET << C_411 "- CLOSED -\n";
                it = pop_connec(it);
                std::cerr << C_330 "\n-----------------------------------------]" << std::endl;
            }
            else
                ++it;
        }
    }
}

#include <string.h>
///////////////////////////////////////////////////////////////////////////////]
void    Server::run_simple( void ) {

    while (true) {
std::cout << C_431 "waiting on accept()" RESET << std::endl;

        struct sockaddr_in  client_addr;
        socklen_t           addr_len;
        int client_fd = accept(_socket_fd, (struct sockaddr*)&client_addr, &addr_len);

        if (client_fd < 0) {
            printErr(RED "accept() failed" RESET);
            continue;
        }
std::cout << C_431 "\taccepted..." RESET << std::endl;


        Connection    request(client_fd, client_addr, addr_len);
        _clients[client_fd] = request;
std::cout << C_241 "\trequest created..." RESET << std::endl;

std::cout << C_241 "\treading..." RESET << std::endl;
        // int r = request.recv_all_buffer();
// std::cout << C_241 "\tfinished, status: " RESET << r << std::endl;

std::cout << C_241 "\tprinting buffer:\n" RESET << request._buffer << std::endl;

std::cout << C_253 "\tsending answer..." RESET << std::endl;

        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
        send(request._client_fd, response, strlen(response), 0);


        close(request._client_fd);
std::cout << C_253 "\tfinished..." RESET << std::endl;
    }
}