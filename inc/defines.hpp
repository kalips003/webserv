#ifndef DEFINES_HPP
#define DEFINES_HPP

#include "_colors.h"

#define DEBUG_MODE false


// #define BUFFER_SIZE 4096
#define BUFFER_SIZE 8192

#define HOW_MANY_REQUEST_PER_LISTEN 2000

#define MAX_BODY_SIZE   8192
#define MAX_LIMIT_FOR_HEAD 1000
#define MAX_LIMIT_FOR_HEADERS 10000
#define MAX_EVENTS 50

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#include <map>
#include <vector>

#define INFO "[" C_150 " INFO  " RESET "] "
#define ERROR "[" C_510 " ERROR " RESET "] "
#define WARNING "[" C_441 "WARNING" RESET "] "
#define DEBUG "[" C_124 " DEBUG " RESET "] "
#define LOG "[" C_035 "  L0G  " RESET "] "


// enum    ConnectionStatus {

//     FIRST = 0,
//     READING_HEADER,
//     READING_BODY,
//     DOING,
//     SENDING,
//     CLOSED
// };

enum    AnswerStatus {

    SENDING_HEAD = 0,
    SENDING_BODY,
    SENDING_BODY_FD,
    ENDED
};

enum BodyMode {

	BODY_NONE, // → GET/HEAD, nothing to read
	BODY_CONTENT_LENGTH, // → read fixed size
	BODY_CHUNKED, // → chunked transfer
	BODY_MULTIPART // → form/file uploads
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


typedef std::ostringstream oss;
typedef std::pair<std::string, std::string> kv;
typedef std::map<std::string, std::string> map_strstr;
typedef std::map<std::string, std::string> map_istr;




#endif
