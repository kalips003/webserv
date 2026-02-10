#ifndef DEFINES_HPP
#define DEFINES_HPP

#include "_colors.h"

#define BUFFER_SIZE 512

#define HOW_MANY_REQUEST_PER_LISTEN 2000

#define MAX_BODY_SIZE   8192
#define MAX_LIMIT_FOR_HEAD 1000
#define MAX_LIMIT_FOR_HEADERS 8192
#define MAX_EVENTS 50

#ifndef PATH_MAX
#define PATH_MAX 1024
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
