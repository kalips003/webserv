
#include <iostream>

#include <stdio.h> // perror
#include <errno.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "_colors.h"

#include "webserv.hpp"
#include "server.hpp"

#include "defines.hpp"
#include "structs.hpp"
///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
void    Server::run_simple( void ) {

    while (true) {

        http_request_wrapper    request;

        request.client_fd = accept(_socket_fd, (struct sockaddr*)&request.client_addr, &request.addr_len);
        if (request.client_fd < 0) {
            printErr(RED "accept() failed" RESET);
            continue;
        }


//  next steps to write:
        // recv_req(request);
        // parse_req();
        // http_request answer = exec_req();
        // send_request(anser);
        // close(request.client_fd);

    }
}
///////////////////////////////////////////////////////////////////////////////]
void    recv_req(http_request_wrapper& request) {

    while (1) {

        char    buffer[BUFFER_SIZE];
        ssize_t bytes_received = recv(request.client_fd, buffer, sizeof(buffer) - 1, 0);
            // possible flags:
// MSG_PEEK:        read data without removing it from queue	(sometimes used to “peek” at headers)
// MSG_DONTWAIT:	non-blocking recv regardless of socket mode	(only if you want to manually control blocking)
// MSG_WAITALL:	    block until full buffer filled	(not good for HTTP; you don’t know body size upfront)
// MSG_NOSIGNAL:	prevent SIGPIPE on Linux	(can be handy when writing to closed socket, but not for recv)
        
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



        request.bytes_received += bytes_received;
    }
}

void    find_delimit(http_request_wrapper& request, char* buff) {

    const char* delim = "\r\n\r\n";

    while (*buff != '\0') {
        if (*buff == delim[request.delim_status])
            request.delim_status++;
        else
            request.delim_status = (*buff == delim[0]);
        if (request.delim_status == strlen(delim)) {

            bool s = request.request.parse_header(request.msg.substr(0, request.msg.size() - 3));
            request.request.body += buff;
            request.status++;

// wrong here +1 -1 somewhere



        }

        request.msg.push_back(*buff);
        buff++;
    }
}


///////////////////////////////////////////////////////////////////////////////]
void    run_simple_plus_notes( void ) {

    while (true) {
// Give me one fully established connection from the queue, and create a new file descriptor for it.
    // blocks by default
        // Non-blocking mode:
        //      set listening socket with fcntl(sockfd, F_SETFL, O_NONBLOCK)
        //      → accept() returns immediately: -1 and errno = EAGAIN if no connection is waiting
        struct sockaddr_in  client_addr; // can be NULL if not needed
        socklen_t addr_len = sizeof(client_addr); // same
        int client_fd = accept(_socket_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0)
            return printErr(std::string(RED "accept() failed" RESET));

//  ACTUAL CODE:
// ssize_t recv(int sockfd, void *buf, size_t len, int flags);
    // recv(client_fd, buffer, size, 0);  // read from client
// ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    // send(client_fd, buffer, size, 0);  // send response

// ...

        close(client_fd);
    }

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