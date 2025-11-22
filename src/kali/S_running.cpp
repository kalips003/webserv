#include "webserv.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include "Server.hpp"

#include <fcntl.h>

typedef std::map<int, connection>::iterator c_it;

#include <cerrno>
///////////////////////////////////////////////////////////////////////////////]
void    Server::accept_client() {

    struct sockaddr_in  client_addr;
    socklen_t           addr_len = sizeof(client_addr); 

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

///////////////////////////////////////////////////////////////////////////////]
void    Server::run_better( void ) {

    char buffer[4096];

    while (true) {

        accept_client();
    
        for (c_it it = _clients.begin(); it != _clients.end(); ) {

            connection &connec = it->second;
            
            if (connec._status <= READING_BODY) {
                std::cerr << C_515 "-----------------------------------------]\n";
                std::cerr << connec << C_515 "\n\tstatus: " RESET << C_411 "- READING -\n";
                connec._status = connec.ft_read(buffer, sizeof(buffer));
                std::cerr << C_515 "-----------------------------------------]" << std::endl;
            }

            if (connec._status == DOING) {
                std::cerr << C_512 "-----------------------------------------]\n";
                std::cerr << connec << C_512 "\n\tstatus: " RESET << C_411 "- DOING -\n";
                connec._status = connec.ft_doing(); // parsing
                std::cerr << C_512 "-----------------------------------------]" << std::endl;
            }
            
            if (connec._status == SENDING) {
                std::cerr << C_431 "-----------------------------------------]\n";
                std::cerr << connec << C_431 "\n\tstatus: " RESET << C_411 "- SENDING -\n";
                connec._status = connec.ft_send(buffer, sizeof(buffer));
                std::cerr << C_431 "-----------------------------------------]" << std::endl;
            }

            if (connec._status == CLOSED) {
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


        connection    request(client_fd, client_addr, addr_len);
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