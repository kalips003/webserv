#include "webserv.hpp"

#include <unistd.h>
#include <cstring>
///////////////////////////////////////////////////////////////////////////////]
// v 1.0
/*
enum ConnectionStatus connection::ft_send(char *buff, size_t sizeofbuff) {

    if (_status == SENDING_HEADER) {

        size_t remaining = _answer._header_size - _answer._bytes_sent;
        size_t to_copy = remaining < sizeofbuff ? remaining : sizeofbuff;
        memcpy(buff, _answer.version.c_str() + _answer._bytes_sent, to_copy);

        ssize_t bytes_sent = send(_client_fd, buff, to_copy, 0);
        if (bytes_sent <= 0) {
            if (bytes_sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                return SENDING_BODY;
            return CLOSED;
        }
        _answer._bytes_sent += bytes_sent;
        if (_answer._bytes_sent >= _answer._header_size) {
            _answer._bytes_sent = 0;
            return static_cast<ConnectionStatus>(SENDING_BODY + !_answer._full_size);
        }
        return SENDING_HEADER;
    }

    if (_status == SENDING_BODY && _answer.fd_body < 0) {

        size_t remaining = _answer._full_size - _answer._bytes_sent;
        size_t to_copy = remaining < sizeofbuff ? remaining : sizeofbuff;
        memcpy(buff, _answer.version.c_str() + _answer._bytes_sent, to_copy);

        ssize_t bytes_sent = send(_client_fd, buff, to_copy, 0);
        if (bytes_sent <= 0) {
            if (bytes_sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                return SENDING_BODY;
            return CLOSED;
        }
        _answer._bytes_sent += bytes_sent;
        if (_answer._bytes_sent >= _answer._full_size)
            return CLOSED;
        return SENDING_BODY;
    }

    if (_status == SENDING_BODY && _answer.fd_body >= 0) {
        if (!_answer.head.empty()) {
            // copy body into buffer first
            _answer.head = "";
        }

        size_t remaining = _answer._full_size - _answer._bytes_sent;
        size_t to_copy = remaining < sizeofbuff ? remaining : sizeofbuff;
        ssize_t bytes_loaded = read(_answer.fd_body, buff, to_copy);
        if (bytes_loaded < 0)
            ; //error
     
        ssize_t bytes_sent = send(_client_fd, buff, bytes_loaded, 0);
        if (bytes_sent <= 0) {
            if (bytes_sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                return SENDING_BODY;
            return CLOSED;
        }   
        if (bytes_sent < bytes_loaded) {
            _answer.head = std::string(buff + bytes_sent, bytes_loaded - bytes_sent);
        }
        _answer._bytes_sent += bytes_sent;

        if (_answer._bytes_sent >= _answer._full_size)
            return CLOSED;
    }

    return static_cast<ConnectionStatus>(_status);
}
*/

// v 1.0
/*
enum ConnectionStatus connection::ft_send(char *buff, size_t sizeofbuff) {

    if (!_answer.head.empty()) {
//  [ /////////// ] /////////////////////////////////////// ]
        size_t remaining = _answer._full_size - _answer._bytes_sent;
        size_t to_copy = remaining < sizeofbuff ? remaining : sizeofbuff;
        memcpy(buff, _answer.version.c_str() + _answer._bytes_sent, to_copy);

        ssize_t bytes_sent = send(_client_fd, buff, to_copy, 0);
        if (bytes_sent <= 0) {
            if (bytes_sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                return SENDING;
            return CLOSED;
        }
        _answer._bytes_sent += bytes_sent;
        if (_answer._bytes_sent == _answer.head.size())
            _answer.head.clear(); // 
    }
    else {
        if (!_answer.body_leftover.empty()) {
//  [ /////////// ] /////////////////////////////////////// ]

            memcpy(buff, _answer.body_leftover.c_str(), _answer.body_leftover.size()); // load and send leftover
            ssize_t bytes_sent = send(_client_fd, buff, _answer.body_leftover.size(), 0);
            if (bytes_sent <= 0) {
                if (bytes_sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    return SENDING;
                return CLOSED;
            }
            if (bytes_sent < _answer.body_leftover.size()) {
                buff[_answer.body_leftover.size()] = '\0';
                _answer.body_leftover = std::string(buff + bytes_sent);
            }
            else
                _answer.body_leftover.clear();
            _answer._bytes_sent += bytes_sent;
        }
        else if (_answer.fd_body >= 0) {
//  [ /////////// ] /////////////////////////////////////// ]
            size_t remaining = _answer._full_size - _answer._bytes_sent;
            size_t to_copy = remaining < sizeofbuff ? remaining : sizeofbuff;
            ssize_t bytes_loaded = read(_answer.fd_body, buff, to_copy);
            if (bytes_loaded < 0)
                ; //error
        
            ssize_t bytes_sent = send(_client_fd, buff, bytes_loaded, 0);
            if (bytes_sent <= 0) {
                if (bytes_sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    return SENDING;
                return CLOSED;
            }   
            if (bytes_sent < bytes_loaded) {
                _answer.head = std::string(buff + bytes_sent, bytes_loaded - bytes_sent);
            }
            _answer._bytes_sent += bytes_sent;
        }
    }
    if (_answer._bytes_sent >= _answer._full_size)
        return CLOSED;
    return SENDING;
}
*/


#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

///////////////////////////////////////////////////////////////////////////////]
bool    f_connect();
void    f_loop_simple(int fd_socket);
///////////////////////////////////////////////////////////////////////////////]
// Lets be honest, this is notes
// Single use recieve stuff;
// works on port:
#define CPORT 8080
///////////////////////////////////////////////////////////////////////////////]
bool    f_connect() {

    int fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_socket < 0)
        return printErr(RED "socket() failed" RESET);

    struct sockaddr_in  addr;
    int                 c_port = CPORT;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(c_port);
    addr.sin_addr.s_addr = INADDR_ANY; //  or: inet_addr("192.168.1.100");


    // in case of rapid on/off of the server (TIME_WAIT state), avoid the EADDRINUSE bind error
    int opt = 1;
    setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int bind_status = bind(fd_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (!!bind_status)
        return printErr(RED "bind() failed" RESET);
            // most common because port already in use: EADDRINUSE
            // or wrong IPaddr, or permission issue: EACCES (port < 1024 = privileged port)

// listen() marks the socket as ready to recieve
// connect() marks the socket as ready to send/initiate
    int how_many_request = 4;
    // how many client can tried to connect to this socket while i call accept()
    int listen_status = listen(fd_socket, how_many_request);
    // 3 step handshake: SYN > SYN-ACK > ACK
    if (!!listen_status)
        return printErr(RED "listen() failed" RESET);

// Give me one fully established connection from the queue, and create a new file descriptor for it.
    // blocks by default
        // Non-blocking mode:
        //      set listening socket with fcntl(sockfd, F_SETFL, O_NONBLOCK)
        //      → accept() returns immediately: -1 and errno = EAGAIN if no connection is waiting
    struct sockaddr_in  client_addr; // can be NULL if not needed
    socklen_t addr_len = sizeof(client_addr); // same
    int client_fd = accept(fd_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0)
        return printErr(RED "accept() failed" RESET);

//  ACTUAL SENDING / RECEIVING:
    char buffer[4096];
// ssize_t recv(int sockfd, void *buf, size_t len, int flags);
// | flag                 | meaning                                                                                         |
// | -------------------- | ----------------------------------------------------------------------------------------------- |
// | **0**                | default — blocking read until data or connection closed                                         |
// | **MSG_PEEK**         | read data *without removing* it from the socket buffer (next `recv()` sees the same data again) |
// | **MSG_WAITALL**      | block until **exactly `len` bytes** have been received (unless error or connection close)       |
// | **MSG_DONTWAIT**     | make this single `recv()` **non-blocking**, even if socket itself is blocking                   |
// | **MSG_TRUNC**        | (datagram sockets only) — return the **actual size** of the message, even if it was truncated   |
// | **MSG_NOSIGNAL**     | prevent `SIGPIPE` if the connection is closed — instead `recv()` just fails with `EPIPE`        |
// | **MSG_ERRQUEUE**     | (Linux-specific) — read from the socket’s **error queue** (e.g., ICMP errors)                   |
// | **MSG_CMSG_CLOEXEC** | (Linux 2.6.23+) — set close-on-exec flag for received file descriptors (used with `recvmsg()`)  |
    ssize_t bit_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bit_received == 0)
        return !printErr(GREEN "client closed connection cleanly (FIN received)" RESET);
    else if (bit_received < 0)
        return printErr(RED "recv() failed" RESET);
    buffer[bit_received] = '\0';
// 
    std::cout << C_025 "Message received from: " RESET
        << "placeholder" << std::endl;
    std::cout << C_431 << buffer << RESET << std::endl;
// 
    const char* answer = "For the millionth time, I AM MOT INTERESTED!";
// ssize_t send(int sockfd, const void *buf, size_t len, int flags);
// | flag              | works with            | meaning                                                                           |
// | ----------------- | ----------------------| --------------------------------------------------------------------------------- |
// | **0**             | default — blocking send until data is queued or error |                                                                                   |
// | **MSG_DONTWAIT**  | all sockets           | make this call **non-blocking** (returns `EAGAIN` if it would block)              |
// | **MSG_NOSIGNAL**  | Linux only            | prevent `SIGPIPE` if peer closed connection; error `EPIPE` instead                |
// | **MSG_MORE**      | TCP (Linux)           | tells the kernel “more data is coming soon”, so delay sending to coalesce packets |
// | **MSG_EOR**       | stream sockets        | mark this as **end of record** (rare; used in record-based protocols)             |
// | **MSG_CONFIRM**   | datagram (UDP/RAW)    | used to confirm reachability (for ARP/NDP); not used for TCP                      |
// | **MSG_DONTROUTE** | datagram or raw       | send packet without routing — stays on local network only                         |
    ssize_t bit_sent = send(client_fd, answer, strlen(answer), 0);
    if (bit_sent < 0)
        return printErr(RED "send() failed" RESET);

    close(client_fd);
    close(fd_socket);

    return true;
}

///////////////////////////////////////////////////////////////////////////////]
// just a loop for acccept
// this function is to be called once listen() is finished
void    f_loop_simple(int fd_socket) {

    while (true)
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int client_fd = accept(fd_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            printErr(RED "accept() failed" RESET);
            return ;
            continue;
        }

        char buffer[4096];
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            close(client_fd);
            printErr(RED "recv() failed or connection closed" RESET);
            return ; // client disconnected or error
            continue; // client disconnected or error
        }
        buffer[bytes] = '\0';
// 
        std::cout << C_025 "Message received from: " RESET
            << "placeholder" << std::endl;
        std::cout << C_431 << buffer << RESET << std::endl;
// 
        // optional: build your response
        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
        send(client_fd, response, strlen(response), 0);

        close(client_fd);
    }
}
