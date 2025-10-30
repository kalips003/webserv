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
    Opens a directory stream.

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
Request:
GET /index.html HTTP/1.1
Host: example.com
User-Agent: curl/8.0
...

Response:
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1234
...

<html> ... </html>
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

send()              - C / POSIX
    Sends data on socket (after writable event).

recv()              - C / POSIX
    Receives data from socket (after readable event).

setsockopt()        - C / POSIX
    Configures socket options (e.g., SO_REUSEADDR).

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

poll()              - C / POSIX
    Monitors multiple FDs for readiness (read/write).

select()            - C / POSIX
    Alternative to poll(), uses FD_SET macros.

epoll_create()      - C / Linux
    Creates epoll instance (Linux only).

epoll_ctl()         - C / Linux
    Controls epoll (add/remove/modify monitored FDs).

epoll_wait()        - C / Linux
    Waits for events on epoll instance.

kqueue()            - C / BSD / macOS
    Creates a kqueue event notification interface.

kevent()            - C / BSD / macOS
    Waits for or submits events to kqueue.


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
7. C++ 98 STANDARD LIBRARY (ALLOWED)
============================================================

- std::string, std::map, std::vector, std::deque, std::list
- std::ifstream, std::ofstream, std::ostringstream, std::istringstream
- std::exception, std::runtime_error
- iostream, sstream, algorithm, utility, iomanip
- new, delete
- STL containers and algorithms from C++98 (no C++11 features)

Used for: configuration parsing, data storage, logic structuring, error handling.


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
