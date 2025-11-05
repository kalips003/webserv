
#include <iostream>

#include <stdio.h> // perror
#include <errno.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "_colors.h"

#include "structs.hpp"
///////////////////////////////////////////////////////////////////////////////]
int printErr(std::string errmsg) {

    perror(errmsg.c_str());
    std::cout << ERR0 << errmsg << std::endl;
    std::cout << C_412 "ERRNO: " RESET << errno << std::endl;
    return 1;
}

int printErr(const char* errmsg) {

    perror(errmsg);
    std::cout << ERR0 << errmsg << std::endl;
    std::cout << C_412 "ERRNO: " RESET << errno << std::endl;
    return 1;
}
///////////////////////////////////////////////////////////////////////////////]
int    f_connect() {

    int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_socket < 0)
        return printErr(std::string(RED "socket() failed" RESET));

    struct sockaddr_in  addr;
    int                 c_port = 8080;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(c_port);
    addr.sin_addr.s_addr = INADDR_ANY; //  or: inet_addr("192.168.1.100");


    // in case of rapid on/off of the server (TIME_WAIT state), avoid the EADDRINUSE bind error
    int opt = 1;
    setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int bind_status = bind(fd_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (!!bind_status)
        return printErr(std::string(RED "bind() failed" RESET));
            // most common because port already in use: EADDRINUSE
            // or wrong IPaddr, or permission issue: EACCES (port < 1024 = privileged port)

// listen() marks the socket as ready to recieve
// connect() marks the socket as ready to send/initiate
    int how_many_request = 4;
    // how many client can tried to connect to this socket while i call accept()
    int listen_status = listen(fd_socket, how_many_request);
    // 3 step handshake: SYN > SYN-ACK > ACK
    if (!!listen_status)
        return printErr(std::string(RED "listen() failed" RESET));

// Give me one fully established connection from the queue, and create a new file descriptor for it.
    // blocks by default
        // Non-blocking mode:
        //      set listening socket with fcntl(sockfd, F_SETFL, O_NONBLOCK)
        //      → accept() returns immediately: -1 and errno = EAGAIN if no connection is waiting
    struct sockaddr_in  client_addr; // can be NULL if not needed
    socklen_t addr_len = sizeof(client_addr); // same
    int client_fd = accept(fd_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0)
        return printErr(std::string(RED "accept() failed" RESET));

//  ACTUAL CODE:
// ssize_t recv(int sockfd, void *buf, size_t len, int flags);
    // recv(client_fd, buffer, size, 0);  // read from client
// ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    // send(client_fd, buffer, size, 0);  // send response

// ...

    close(client_fd);

    close(fd_socket);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////]
void    f_loop_simple(int fd_socket) {

    while (true)
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int client_fd = accept(fd_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        char buffer[4096];
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            close(client_fd);
            continue; // client disconnected or error
        }

        buffer[bytes] = '\0';
        std::cout << "Got request:\n" << buffer << std::endl;

        // optional: build your response
        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
        send(client_fd, response, strlen(response), 0);

        close(client_fd);
    }
}

#define BUFFER_SIZEE 1025
///////////////////////////////////////////////////////////////////////////////]
http_request_wrapper    recv_req(int client_fd) {

    http_request_wrapper    msg;
    while (1) {

        char    buffer[BUFFER_SIZEE];
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            // possible flags:
        
        if (bytes_received == 0) {
            // client closed connection cleanly (FIN received)
            break;
        }
        else if (bytes_received < 0) {// treat as generic fail (no errno)
            // error (EAGAIN, etc.)
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else if (errno == EINTR)
                continue; // interrupted by signal, retry
            else
                perror("recv");
            break;
        }
// do some preparsing to detect the end of headers (\r\n\r\n)
// if Content-Length: N, read N bytes and dont wiat for the return of recv = 0
        buffer[bytes_received] = '\0';
        msg.msg += buffer;
    }
    return msg;
}