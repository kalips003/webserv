============================================================
1. PROCESS CONTROL
============================================================

fork()              - C / POSIX
    Duplicates current process. Only allowed for CGI execution.
    Returns 0 in child, PID in parent.

execve()            - C / POSIX
    Executes a new program in the current process (used for CGI).
    Replaces current process image with new executable.

waitpid()           - C / POSIX
    Waits for a specific child process to terminate (reap CGI child).

kill()              - C / POSIX
    Sends a signal to a process (e.g., SIGTERM).
    May be used to terminate CGI on timeout.

signal()            - C / POSIX
    Sets signal handler (e.g., ignore SIGPIPE to prevent crash on closed socket).


============================================================
2. FILE DESCRIPTORS AND GENERAL I/O
============================================================

open()              - C / POSIX
    Opens a file, returns file descriptor.

close()             - C / POSIX
    Closes an open file descriptor.

read()              - C / POSIX
    Reads from file descriptor (must only be done after readiness via poll/select).

write()             - C / POSIX
    Writes to file descriptor (must only be done after readiness).

dup()               - C / POSIX
    Duplicates a file descriptor.

dup2()              - C / POSIX
    Duplicates FD to a specific target descriptor (used for CGI redirection).

fcntl()             - C / POSIX
    Controls file descriptor flags.
    Allowed flags: F_SETFL, O_NONBLOCK, FD_CLOEXEC.

pipe()              - C / POSIX
    Creates a pair of connected file descriptors.
    Used for interprocess communication (e.g., CGI stdin/stdout).

access()            - C / POSIX
    Checks file existence or permission.

stat()              - C / POSIX
    Retrieves file status (size, type, modification time).

opendir()           - C / POSIX
    Opens a directory stream.better, but still linear scan

readdir()           - C / POSIX
    Reads directory entries.

closedir()          - C / POSIX
    Closes directory stream.


============================================================
3. SOCKET PROGRAMMING:
    Server Workflow

socket() → create a listening socket.
setsockopt() (optional) → configure options like SO_REUSEADDR.
bind() → assign an IP address and port to the socket.
listen() → mark the socket as passive to accept incoming connections.
accept() → accept a client connection; returns a new socket FD for communication.
recv() / send() → exchange data with the client.
close() → close the client socket; optionally continue to accept more clients.
close() → close the listening socket when shutting down.

    Client Workflow

socket() → create a socket for outgoing connection.
connect() → connect to the server's IP and port.
send() / recv() → exchange data with the server.
close() → close the connection when done.
========================================================================================================================
<!--                    socket()  -->
<!-- Creates a new socket descriptor. -->
```cpp

int sockfd = socket(int domain, int type, int protocol) {
    // This defines the type of addresses you’ll use (IPv4, IPv6, local, etc).
    domain = AF_INET = IPv4 (HTTP, TCP, UDP)
            = AF_INET6 = IPv6
            = AF_UNIX / AF_LOCAL = Local (inter-process) communication on the same machince = Unix domain sockets
            = AF_PACKET = Low-level access to network devices (raw Ethernet frames) = (Sniffers, ARP tools)
            = AF_NETLINK = Kernel-user space communication in Linux = Routing table updates
            = AF_BLUETOOTH = Bluetooth sockets = Bluetooth apps
            = AF_UNSPEC = Unspecified — used for lookups (not for creating actual sockets)
    // Defines how data is sent (reliable stream, datagrams, raw packets...).
    type = SOCK_STREAM > TCP || SOCK_DGRAM > UDP
            = SOCK_STREAM = Stream socket :	TCP (connection-oriented, reliable, ordered, byte stream)
            = SOCK_DGRAM = Datagram socket :	UDP (connectionless, unreliable, packet-based)
            = SOCK_RAW = Raw socket :	Direct access to lower-level protocols (requires root)
            = SOCK_SEQPACKET = Sequential packet socket :	Like stream but message boundaries preserved
            = SOCK_RDM = Reliable datagram (rarely used) :	Reliable, but message-based
        // Flags (can be combined with bitwise OR):
            SOCK_NONBLOCK	Create non-blocking socket immediately
            SOCK_CLOEXEC	Close socket automatically on exec() (security/cleanup)
    // Specific protocol in the family (usually = 0): The kernel chooses the default protocol that matches domain + type
    protocol = 0
        // But you can explicitly specify:
            IPPROTO_TCP (6), domain: AF_INET, type: SOCK_STREAM : TCP
            IPPROTO_UDP (17), domain: AF_INET, type: SOCK_DGRAM : UDP
            IPPROTO_ICMP (1), domain: AF_INET, type: SOCK_RAW : ICMP (ping)
            IPPROTO_RAW (255), domain: AF_INET, type: SOCK_RAW : Direct IP packets
}
int sockfd = socket(AF_INET, SOCK_STREAM, 0);

```

Application	HTTP, FTP, SMTP, DNS
    Defines what the data means, the language of the message
Transport	TCP, UDP
    Defines how to send it reliably or not, message boundaries, order, error checking
Internet / Network	IP (IPv4, IPv6)
    Defines where to send it, addresses and routing

Transport layer:
TCP (Transmission Control Protocol) = a registered mail with tracking: everything must arrive in order.
    Used for: HTTP, HTTPS, FTP, SSH.
UDP (User Datagram Protocol) = a postcard: it may get lost, but you don’t care much.
    Used for: DNS queries, streaming, VoIP, gaming.

Application layer: Defines the format of messages between client and server.
HTTP, structure:

```yaml
POST /api/user HTTP/1.1
Host: example.com
Content-Type: application/json
Content-Length: 38

{
  "name": "Alice",
  "age": 30
}
```

An HTTP request has three main parts:
<Request Line>
    First line of the request: method, path, version
        . what you want to do (GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS, CONNECT, TRACE)
        . Path: resource on the server (/index.html, /api/users)
        . Version: HTTP version (HTTP/1.0, HTTP/1.1, HTTP/2)
<Headers>
    Key:Value pairs providing metadata about the request.
    Each header is on its own line.
    Ends with an empty line (\r\n) to separate headers from the body.
        . Host	Mandatory in HTTP/1.1, specifies the server domain
        . User-Agent	Identifies the client software
        . Accept	Which content types the client can handle
        . Content-Type	Type of the body (for POST/PUT requests)
        . Content-Length	Size of the body in bytes
<Body (optional)>
ex:

HTTP Response structure, also three main parts:
<Status Line> = HTTP/1.1 200 OK (\r\n)
<Headers>                       (\r\n)
                                (empty line separating) (\r\n\r\n)
<Body (optional)>

STATUS:
Version: HTTP version

Status code: numeric, indicates success/failure

    2xx → success (200 OK, 201 Created)
    3xx → redirection (301 Moved Permanently)
    4xx → client error (404 Not Found)
    5xx → server error (500 Internal Server Error)

// --- 1xx: Informational ---
#define HTTP_CONTINUE                     100
#define HTTP_SWITCHING_PROTOCOLS          101
#define HTTP_PROCESSING                   102
#define HTTP_EARLY_HINTS                  103

// --- 2xx: Success ---
#define HTTP_OK                           200
#define HTTP_CREATED                      201
#define HTTP_ACCEPTED                     202
#define HTTP_NON_AUTHORITATIVE_INFO       203
#define HTTP_NO_CONTENT                   204
#define HTTP_RESET_CONTENT                205
#define HTTP_PARTIAL_CONTENT              206
#define HTTP_MULTI_STATUS                 207
#define HTTP_ALREADY_REPORTED             208
#define HTTP_IM_USED                      226

// --- 3xx: Redirection ---
#define HTTP_MULTIPLE_CHOICES             300
#define HTTP_MOVED_PERMANENTLY            301
#define HTTP_FOUND                        302
#define HTTP_SEE_OTHER                    303
#define HTTP_NOT_MODIFIED                 304
#define HTTP_USE_PROXY                    305
#define HTTP_TEMPORARY_REDIRECT           307
#define HTTP_PERMANENT_REDIRECT           308

// --- 4xx: Client Errors ---
#define HTTP_BAD_REQUEST                  400
#define HTTP_UNAUTHORIZED                 401
#define HTTP_PAYMENT_REQUIRED             402
#define HTTP_FORBIDDEN                    403
#define HTTP_NOT_FOUND                    404
#define HTTP_METHOD_NOT_ALLOWED           405
#define HTTP_NOT_ACCEPTABLE               406
#define HTTP_PROXY_AUTH_REQUIRED          407
#define HTTP_REQUEST_TIMEOUT              408
#define HTTP_CONFLICT                     409
#define HTTP_GONE                         410
#define HTTP_LENGTH_REQUIRED              411
#define HTTP_PRECONDITION_FAILED          412
#define HTTP_CONTENT_TOO_LARGE            413   // or REQUEST_ENTITY_TOO_LARGE
#define HTTP_URI_TOO_LONG                 414
#define HTTP_UNSUPPORTED_MEDIA_TYPE       415
#define HTTP_RANGE_NOT_SATISFIABLE        416
#define HTTP_EXPECTATION_FAILED           417
#define HTTP_IM_A_TEAPOT                  418
#define HTTP_MISDIRECTED_REQUEST          421
#define HTTP_UNPROCESSABLE_ENTITY         422
#define HTTP_LOCKED                       423
#define HTTP_FAILED_DEPENDENCY            424
#define HTTP_TOO_EARLY                    425
#define HTTP_UPGRADE_REQUIRED             426
#define HTTP_PRECONDITION_REQUIRED        428
#define HTTP_TOO_MANY_REQUESTS            429
#define HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE 431
#define HTTP_UNAVAILABLE_FOR_LEGAL_REASONS 451

// --- 5xx: Server Errors ---
#define HTTP_INTERNAL_SERVER_ERROR        500
#define HTTP_NOT_IMPLEMENTED              501
#define HTTP_BAD_GATEWAY                  502
#define HTTP_SERVICE_UNAVAILABLE          503
#define HTTP_GATEWAY_TIMEOUT              504
#define HTTP_HTTP_VERSION_NOT_SUPPORTED   505
#define HTTP_VARIANT_ALSO_NEGOTIATES      506
#define HTTP_INSUFFICIENT_STORAGE         507
#define HTTP_LOOP_DETECTED                508
#define HTTP_NOT_EXTENDED                 510
#define HTTP_NETWORK_AUTH_REQUIRED        511

Reason phrase: human-readable (optional, ignored by computers)

HEADERS:

1️⃣ HTTP Request Headers
Accept:                 MIME types the client can handle (text/html, application/json)
Accept-Charset:         Character sets the client can handle (utf-8, iso-8859-1)
Accept-Encoding:        Compression formats the client accepts (gzip, deflate, br)
Accept-Language:        Preferred languages (en-US, fr)
Authorization:          Credentials for HTTP authentication (Basic, Bearer tokens)
Cache-Control:          Client caching rules (no-cache, max-age=0)
Connection:             keep-alive or close
Content-Length:         Length of the request body in bytes
Content-Type:           MIME type of the request body (application/json, multipart/form-data)
Cookie:                 Cookies sent to the server
Date:                   Date and time of the request
Expect:                 Indicates expectations, e.g., 100-continue
From:                   Email of the user making the request (rare)
Host:                   Mandatory in HTTP/1.1, server domain
If-Match:               Conditional request based on ETag
If-Modified-Since:      Only send if resource changed since date
If-None-Match:          Only send if ETag does not match
If-Range:               Used for partial content requests
If-Unmodified-Since:    Only send if resource not modified since date
Max-Forwards:           Limit for proxies and TRACE requests
Origin:                 Origin of cross-site request (CORS)
Pragma:                 Legacy caching instructions (no-cache)
Proxy-Authorization:    Credentials for proxy authentication
Range:                  Request part of a resource (bytes=0-499)
Referer:                URL of the page making the request
TE:                     Transfer encodings the client accepts
Upgrade:                Request protocol upgrade (websocket)
User-Agent:             Software making the request (browser, curl)
Via:                    Proxies the request went through
Warning:                General warnings

1️⃣ HTTP Response Headers
Accept-Ranges:                 Indicates if server supports range requests (bytes)
Age:                           Age of the cached response in seconds
Allow:                         Methods allowed for the resource (GET, POST, OPTIONS)
Cache-Control:                 Caching rules (no-store, private, max-age)
Connection:                     keep-alive or close
Content-Encoding:               Compression format applied (gzip, br)
Content-Language:               Language of the content (en-US)
Content-Length:                 Length of the response body in bytes
Content-Location:               Alternate location for the returned content
Content-Disposition:            How content should be handled (inline, attachment; filename="file.txt")
Content-Type:                   MIME type of response (text/html, application/json)
Date:                           Date/time of response
ETag:                           Unique identifier for version of resource
Expires:                        Expiration date for caching
Last-Modified:                  Last modification date of resource
Location:                       Redirect target URL (3xx responses)
Pragma:                         Legacy caching instructions (no-cache)
Retry-After:                    When to retry (503 maintenance)
Server:                         Server software/version (nginx/1.25)
Set-Cookie:                     Cookie sent to client
Strict-Transport-Security:      HSTS rules (HTTPS only)
Trailer:                        Headers sent after chunked body
Transfer-Encoding:              Encoding applied (chunked)
Upgrade:                        Protocol upgrades (websocket)
Vary:                           Specifies headers that affect caching (Accept-Encoding)
Via:                            Proxy chain
Warning:                        General warnings
WWW-Authenticate:               Challenge for authentication (Basic, Digest)

========================================================================================================================
sockaddr_in: defines the address and port of a socket for IPv4.

Used in bind() (to assign a local address to a socket)
Used in connect() (client socket connecting to server)
Used in accept() (server receives a client address)
    Important: listen() does not need it directly; it just marks the socket as passive.


```cpp
struct sockaddr_in {
    sa_family_t    sin_family; // Address family (AF_INET)
    uint16_t       sin_port;   // Port number (network byte order)
    struct in_addr sin_addr;   // IP address (struct with uint32_t s_addr)
    char           sin_zero[8]; // Padding, usually set to 0
};
    sin_family → must be AF_INET
    sin_port → port number in network byte order → use htons()
    sin_addr → IP address (struct in_addr) → usually set via inet_addr() or INADDR_ANY
    sin_zero → padding to make sockaddr_in same size as sockaddr; usually memset to 0

    htons() → Host to Network Short (converts 16-bit port to big-endian)
    INADDR_ANY → listen on all IP addresses of the machine (0.0.0.0)
    can specify a specific IP:
        server_addr.sin_addr.s_addr = inet_addr("192.168.1.100");
```
============================================================
bind()              - C / POSIX
    Binds socket to address and port.

listen()            - C / POSIX
    Marks socket as passive (ready to accept).

accept()            - C / POSIX
    Accepts incoming connection (returns new socket FD).
    Must be non-blocking.

connect()           - C / POSIX
    Connects socket to remote address (rarely used here).

```cpp
send()              - C / POSIX
    Sends data on socket (after writable event).
ssize_t send(int sockfd, const void *buf, size_t len, int flags);

recv()              - C / POSIX
    Receives data from socket (after readable event).
ssize_t recv(int sockfd, void *buf, size_t len, int flags);


flags	= usually 0; or MSG_DONTWAIT, MSG_PEEK, etc.
return:	number of bytes read/written, or 0 (connection closed), or -1 on error
```

============================================================
setsockopt()        - C / POSIX
    Configures socket options (e.g., SO_REUSEADDR).
```cpp
int setsockopt(
    int sockfd,      // your socket file descriptor
    int level,       // which layer (level) of options we’re touching
    int option_name, // the specific option you want to set
    const void *option_value, // pointer to the value you’re setting
    socklen_t option_len      // size (in bytes) of that value
);
> For this socket (fd_socket),
at the socket level (SOL_SOCKET),
set the option SO_REUSEADDR
to the value stored in opt (which is 1),
whose size is 4 bytes (sizeof(int))

| Option name               | Belongs to    | Meaning                                                    |
| ------------------------- | ------------- | ---------------------------------------------------------- |
| `SO_REUSEADDR`            | `SOL_SOCKET`  | Allow reusing local address/port                           |
| `SO_REUSEPORT`            | `SOL_SOCKET`  | Allow multiple sockets to bind same port (Linux ≥3.9)      |
| `SO_KEEPALIVE`            | `SOL_SOCKET`  | Enable TCP keepalive probes                                |
| `TCP_NODELAY`             | `IPPROTO_TCP` | Disable Nagle’s algorithm (send small packets immediately) |
| `SO_RCVBUF` / `SO_SNDBUF` | `SOL_SOCKET`  | Set receive/send buffer size                               |

```

============================================================
getsockname()       - C / POSIX
    Gets local address info of socket.

getaddrinfo()       - C / POSIX
    Resolves hostname/service to address info.

freeaddrinfo()      - C / POSIX
    Frees memory from getaddrinfo().

getprotobyname()    - C / POSIX
    Gets protocol info by name ("tcp", "udp", etc.).

htons(), htonl()    - C / POSIX
    Host to network byte order conversion (short/long).

ntohs(), ntohl()    - C / POSIX
    Network to host byte order conversion.

socketpair()        - C / POSIX
    Creates a pair of connected sockets (like pipe()).


============================================================
4. EVENT MULTIPLEXING (NON-BLOCKING I/O CORE)
============================================================
```cpp

// epoll(not on mac)
// =============================================
epoll_create()      - C / Linux
int epoll_create(int size);
    Creates epoll instance (Linux only).

// =============================================
epoll_ctl()         - C / Linux
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
    // Controls epoll (add/remove/modify monitored FDs).
    // op is one of: EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL.

struct epoll_event {
    uint32_t events;   // EPOLLIN, EPOLLOUT, EPOLLET, etc.
    epoll_data_t data; // union { void *ptr; int fd; uint32_t u32; uint64_t u64; }
};
// =============================================
epoll_wait()        - C / Linux
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
    Waits for events on epoll instance.
    Blocks until one or more FDs are ready or timeout expires.
    Returns number of ready events, or -1 on error.


// =============================================
fcntl()             - C / POSIX
int fcntl(int fd, int cmd, ... /* arg */ );
ex:
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK); // set non-blocking mode

    Controls file descriptor flags.
    Allowed flags: F_SETFL, O_NONBLOCK, FD_CLOEXEC.
        F_GETFL → get file status flags
        F_SETFL → set file status flags
    Also used for duplication, locking, etc.

```
============================================================

🧩 1️⃣ select() family (oldest)

    universal (POSIX standard — works everywhere)
    but limited by FD_SETSIZE (usually 1024 FDs)
    slow for many connections because it scans every fd each time
    ✅ good for small demos, not for production-level server

🧩 2️⃣ poll() family (better)

    replaces select(), no hard FD limit
    uses an array of pollfd
    still linear scan every loop → O(n) performance
    ✅ fine for dozens/hundreds of clients
    ⚠️ not great for thousands

🧩 3️⃣ epoll() (Linux) / kqueue() (BSD/macOS)

    modern high-performance event notification systems
    you register fds once, and the kernel keeps track internally
    epoll_wait() / kevent() return only the sockets that changed
    O(1) performance (roughly), very scalable
    ✅ used by nginx, redis, node.js, etc.

```cpp
// =============================================
select()            - C / POSIX
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
    Alternative to poll() (oldest version), uses FD_SET macros.
    Monitors sets of FDs for readability/writability/exceptions.
    Uses macros: FD_SET, FD_CLR, FD_ISSET, FD_ZERO.
    Oldest, limited by FD_SETSIZE.
// =============================================
poll()              - C / POSIX
    Monitors multiple FDs for readiness (read/write). (better, but still linear scan)
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
    Scans an array of FDs linearly.
    Timeout in milliseconds (-1 = block forever).

struct pollfd {
    int   fd;        // file descriptor
    short events;    // requested events (e.g. POLLIN, POLLOUT)
    short revents;   // returned events
};
// =============================================
kqueue()            - C / BSD / macOS
int kqueue(void);
    Creates a kqueue event notification interface.
    Creates a kernel event queue (BSD/macOS equivalent of epoll).

// =============================================
kevent()            - C / BSD / macOS
int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);
    Waits for or submits events to kqueue.
    Handles both submitting and waiting for events in one call.

struct kevent {
    uintptr_t ident;   // fd or identifier
    int16_t   filter;  // EVFILT_READ, EVFILT_WRITE, etc.
    uint16_t  flags;   // EV_ADD, EV_DELETE, EV_ENABLE, EV_DISABLE
    uint32_t  fflags;
    intptr_t  data;
    void     *udata;
};

```

============================================================
5. ENVIRONMENT AND ERROR HANDLING
============================================================

strerror()          - C / POSIX
    Converts errno code to human-readable string.

gai_strerror()      - C / POSIX
    Converts getaddrinfo() error to string.

errno               - C / POSIX
    Global variable for last system call error.
    Forbidden for read/write logic decisions.


============================================================
6. MISCELLANEOUS
============================================================

chdir()             - C / POSIX
    Changes current working directory (used before execve in CGI).

============================================================
NOTES
============================================================

- All listed functions are POSIX C system calls.
- All networking and I/O must use non-blocking mode.
- All I/O readiness must be driven by a single poll()/select()/epoll()/kqueue().
- Never rely on errno after read/write.
- fork() allowed only for CGI execution.
- Implementation language: C++98.
- System interaction: C POSIX API.
