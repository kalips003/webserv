#ifndef DEFINES_HPP
#define DEFINES_HPP

#include "_colors.h"

#define BUFFER_SIZE 4096

#define HOW_MANY_REQUEST_PER_LISTEN 2000

// #define MAX_BODY_SIZE   8192 // default if not defined (-1)
#define MAX_LIMIT_FOR_HEAD 1000 // max limit for request's first line
#define MAX_LIMIT_FOR_HEADERS 8192 // max size before CRLFCRLF
#define MAX_EVENTS 50 // max event epoll can handle at a time

#ifndef PATH_MAX
#define PATH_MAX 1024 // max size for current dir path
#endif

#include <map>
#include <vector>

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

typedef std::ostringstream oss;
typedef std::pair<std::string, std::string> kv;
typedef std::map<std::string, std::string> map_strstr;
typedef std::map<std::string, std::string> map_istr;
typedef std::vector<std::string> v_str;

#endif
