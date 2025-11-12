#include "webserv.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

#include "Server.hpp"

#include <fcntl.h>
typedef std::map<int, connection>::iterator c_it;
///////////////////////////////////////////////////////////////////////////////]
void    Server::run_better( void ) {

    char buffer[4096];

    while (true) {

        struct sockaddr_in  client_addr;
        socklen_t           addr_len;
        int client_fd = accept(_socket_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            printErr(RED "accept() failed" RESET);
            continue;
        }

// NON BLOCCKING recv():
//      int     fcntl(int fd, int cmd, ... /* arg */ );
// F_GETFL = get current file status flags (O_NONBLOCK, O_APPEND, ...).
//          return bitmask of flags
// F_SETFL = set flags with 'arg'
//          return ?
// FLAGS:
/*
    O_NONBLOCK	make reads/writes non-blocking
    O_APPEND	always write at the end of file
    O_SYNC	    write operations wait for physical completion
    O_ASYNC	    enable SIGIO delivery when I/O is possible (less used)
*/
        int flags = fcntl(client_fd, F_GETFL, 0);
        if (flags < 0){
            printErr(RED "fcntl( F_GETFL ) failed" RESET);
            continue;
        }
        if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0){
            printErr(RED "accept( F_SETFL ) failed" RESET);
            continue;
        }
///////////////////////////
        _clients[client_fd] = connection(client_fd, client_addr, addr_len);

        for (c_it it = _clients.begin(); it != _clients.end(); it++) {

            int this_pid = it->first;
            connection &connec = it->second;
            
            if (connec._status <= READING_BODY)
                connec.read_buffer(buffer, sizeof(buffer));

            if (connec._status == DOING) {
                bool isRequestOK = connec.checkRequest();
                if (isRequestOK)
                    ft_exec(connec);
                connec._status == SENDING;//?
            }
            
            if (connec._status == SENDING)
                connec.send_buffer(buffer, sizeof(buffer));

            if (connec._status == CLOSED)
                pop_fd(connec._client_fd);
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
        int r = request.recv_all_buffer();
std::cout << C_241 "\tfinished, status: " RESET << r << std::endl;

std::cout << C_241 "\tprinting buffer:\n" RESET << request._buffer << std::endl;

std::cout << C_253 "\tsending answer..." RESET << std::endl;

        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
        send(request._client_fd, response, strlen(response), 0);


        close(request._client_fd);
std::cout << C_253 "\tfinished..." RESET << std::endl;
    }
}