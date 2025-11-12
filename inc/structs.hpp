#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <string>
#include <map>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>

///////////////////////////////////////////////////////////////////////////////]
// name arg { set1 a; set2 b }
struct block {
    std::string                         name;
    std::string                         arg;
    std::map<std::string, std::string>  settings;
};

// name arg
// name arg { set1 a; set2 b }
struct server_settings {
    std::map<std::string, std::string>  global_settings;
    std::vector<block>                  block_settings;
    int                                 port_num;

    server_settings() : port_num(-1) {}
};

///////////////////////////////////////////////////////////////////////////////]
// GET /index.html HTTP/1.1\r\n
// Host: example.com\r\n
// User-Agent: curl/7.81.0\r\n
// Accept: */*\r\n
// \r\n
// <body content if any...>
#include <sstream>
struct http_request {

    std::string method; // GET
    std::string path; // /index.html
    std::string version; // HTTP/1.1

    std::map<std::string, std::string>  headers;

    std::string body;

    int         status_delim; // where are we of "\r\n\r\n"
    ssize_t      body_size;

    http_request() : status_delim(0), body_size(-1) {}
};

///////////////////////////////////////////////////////////////////////////////]
// HTTP/1.1 200 OK
struct http_answer {

    std::string version; // HTTP/1.1
    int         status; // 200
    std::string msg_status; // OK

    std::map<std::string, std::string>  headers;

    std::string body;

    http_answer() : version("HTTP/1"), status(200), msg_status("OK") {}
};

///////////////////////////////////////////////////////////////////////////////]
struct connection {

    int                 _client_fd;
    struct sockaddr_in  _client_addr;
    socklen_t           _addr_len;

    std::string     _buffer;
// _buffer.reserve(8192); // optional

    http_request    _request;
    http_answer     _answer;

    ssize_t         _bytes_received;
    ssize_t         _bytes_sent;

    int             _status; // READING 0 SENDING 1 CLOSED 2

    connection() :
        _client_fd(-1), _addr_len(sizeof(_client_addr)), 
        _bytes_received(0), _bytes_sent(0), _status(0) {}

    connection(int fd, struct sockaddr_in c, socklen_t a) :
        _client_fd(fd), _client_addr(c), _addr_len(a), 
        _bytes_received(0), _bytes_sent(0), _status(0) {}



/*  loop on recv() until all is received from client */
    int     recv_all_buffer();


/*  read from provided buffer once. append to msg
        before append check if delimitor is found in buffer "\r\n\r\n" 
        if delim found, parse it into request, copy the rest into msg, change status to DOING*/
    int     read_buffer(char *buff, size_t sizeofbuff);

/*  check each buffer for the presence of delim "\r\n\r\n" 
        keep in memory where we are at, in _request.status_delim */
    bool    check_buffer(char *buff);

/*  called after the first read, to make sure request is valid "METHOD /path HTTP/1.1" */    
    bool    parse_header_firstline();

/*  true return function, called once "\r\n\r\n" is found
        does the parsing / error handling on the fully downloaded headers */
    bool    parse_header_wrapper(char *buf);

/*  parse _buffer into _request, return false if any pb */
    bool    parse_header();

    bool    create_answer();
    
};

#endif