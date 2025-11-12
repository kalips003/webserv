#ifndef DEFINES_HPP
#define DEFINES_HPP

#define BUFFER_SIZE 1024
#define HOW_MANY_REQUEST_PER_LISTEN 4

#include <map>
#include <vector>

enum    ConnectionStatus {

    FIRST = 0,
    READING_HEADER,
    READING_BODY,
    DOING,
    SENDING,
    CLOSED
};

typedef std::pair<std::string, std::string> kv;
typedef std::map<std::string, std::string> map;

// WebDAV += {PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK}
static const std::string method_array[] = {
    "GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS", "CONNECT", "TRACE"
};

static const std::vector<std::string> HTTP_METHODS(
    method_array,
    method_array + sizeof(method_array) / sizeof(method_array[0])
);





#endif