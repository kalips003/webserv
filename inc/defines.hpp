#ifndef DEFINES_HPP
#define DEFINES_HPP

#define BUFFER_SIZE 1024
#define HOW_MANY_REQUEST_PER_LISTEN 4

#define MAX_BODY_SIZE   8192
#define MAX_LIMIT_FOR_HEAD 1000

#define PATH_MAX   1024

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

enum    AnswerStatus {

    SENDING_HEAD = 0,
    SENDING_BODY,
    SENDING_BODY_FD,
    ENDED
};

enum BodyMode {
    BODY_NONE,
    BODY_CONTENT_LENGTH,
    BODY_CHUNKED
};

enum HttpMethod {
    GET = 0,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS,
    CONNECT,
    TRACE,
    INVALID = -1
};


typedef std::pair<std::string, std::string> kv;
typedef std::map<std::string, std::string> map_strstr;
typedef std::map<std::string, std::string> map_istr;




#endif