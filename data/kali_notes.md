

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
