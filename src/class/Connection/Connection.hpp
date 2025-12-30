#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>

// #include <sys/socket.h>
#include <netinet/in.h>

#include "defines.hpp"
#include "Task.hpp"
#include "HttpAnswer.hpp"
#include "HttpRequest.hpp"
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
class Connection {

private:
///////////////////////////////////////////////////////////////////////////////]
    int                 _client_fd; // fd associated with this client connection
    struct sockaddr_in  _client_addr; // struct with informations about the client
    socklen_t           _addr_len; // ?

    std::string         _buffer;

    HttpRequest         _request;
    httpAnswer          _answer;
    Task                *_body_task;

    int                 _status; // READING 0 SENDING 1 CLOSED 2
///////////////////////////////////////////////////////////////////////////////]

    Connection() :
        _client_fd(-1), _addr_len(sizeof(_client_addr)), 
        _body_task(NULL), _status(FIRST) {}

    Connection(int fd, struct sockaddr_in c, socklen_t a) :
        _client_fd(fd), _client_addr(c), _addr_len(a), 
        _body_task(NULL), _status(FIRST) {}





public:
/*  fills _answer with err_code, */
    enum ConnectionStatus   create_error(int err_code);
///////////////////////////////////////////////////////////////////////////////]
//          FT READ
public:
/**
 * Read from provided buffer once
 *
 * Stores received data into _buffer, keep doing so until the first "\r\n" is found
 * Internally update the _Request object
 *
 *
 * @param buff   Buffer to read from
 * @param sizeofbuff   Size of said buffer
 * @return      enum ConnectionStatus corresponding to the state of the connection after this one read
 */
/*  read from provided buffer once. append to msg
        before append check if delimitor is found in buffer "\r\n\r\n" 
        if delim found, parse it into request, copy the rest into msg, change status to DOING*/
    enum ConnectionStatus   ft_read(char *buff, size_t sizeofbuff);
private:
/*  called after the first read, to make sure request is valid "METHOD /path HTTP/1.1" */    
    enum ConnectionStatus   parse_header_first_read(std::string first_rec);
/*  check each buffer for the presence of delim "\r\n\r\n" 
        keep in memory where we are at, in _request.status_delim */
    enum ConnectionStatus   check_buffer_for_rnrn(char *buff);
/*  true return function, called once "\r\n\r\n" is found
        does the parsing / error handling on the fully downloaded headers */
    enum ConnectionStatus   parse_header_wrapper(char *buf);
/*  parse _buffer into _request, return false if any pb */
    bool                    parse_header_for_syntax();

///////////////////////////////////////////////////////////////////////////////]
//          FT DOING
public:
    enum ConnectionStatus     ft_doing( void ) {
        if (!_body_task)
            return SENDING; // error
        int r = _body_task->ft_do();
        if (r)
            create_error(r);
        else
            _answer.http_answer_ini();
        return SENDING;
    }
///////////////////////////////////////////////////////////////////////////////]
//          FT SEND
public:
    enum ConnectionStatus     ft_send(char *buff, size_t sizeofbuff);
private:

};

std::ostream& operator<<(std::ostream& os, const Connection& c);

#endif