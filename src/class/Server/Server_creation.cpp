#include "Server.hpp"

#include <iostream>

#include "T_tools.cpp"
#include "T_tools2.cpp"
#include "defines.hpp"

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
Server::Server( const char* confi_file ) : _addr(), _socket_fd(-1), _server_status(false) {

	_server_status = _settings.parse_config_file(confi_file);
	if (!_server_status)
		return ;

	_server_status = _settings.check_settings();
	if (!_server_status)
		return ;

	_server_status = create_listening_socket();
	if (!_server_status)
		return ;

	_server_status = true;
	std::cout << C_151 "Server up and running on port: " RESET << _settings.getPortNum() << std::endl;
}

#include <fcntl.h>
///////////////////////////////////////////////////////////////////////////////]
bool	Server::create_listening_socket() {

	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd < 0) {
		return printErr(RED "socket() failed" RESET);
	}
	_addr.sin_port = htons(_settings.getPortNum());
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;

// in case of rapid on/off of the server (TIME_WAIT state), avoid the EADDRINUSE bind error
	int opt = 1;
	setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	int bind_status = bind(_socket_fd, (struct sockaddr *)&_addr, sizeof(_addr));
	if (bind_status) {
	// most common because port already in use: EADDRINUSE
	// or wrong IPaddr, or permission issue: EACCES (port < 1024 = privileged port)
		return printErr(RED "bind() failed" RESET);
	}

// listen() marks the socket as ready to recieve
// connect() marks the socket as ready to send/initiate
	// how many client can tried to connect to this socket while i call accept()
	int listen_status = listen(_socket_fd, HOW_MANY_REQUEST_PER_LISTEN);
	// 3 step handshake: SYN > SYN-ACK > ACK
	if (!!listen_status)
		return printErr(RED "listen() failed" RESET);

	if (!set_flags(_socket_fd, O_NONBLOCK))
		return false;
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
/* 		SOCKET()


int sockfd = socket(int domain, int type, int protocol)

    // This defines the type of addresses you’ll use (IPv4, IPv6, local, etc).
    domain = AF_INET = IPv4 (HTTP, TCP, UDP)
            = AF_INET6 = IPv6
            = AF_UNIX / AF_LOCAL = Local (inter-process) communication on the same machince = Unix domain sockets
            = AF_PACKET = Low-level access to network devices (raw Ethernet frames) = (Sniffers, ARP tools)
            = AF_NETLINK = Kernel-user space communication in Linux = Routing table updates
            = AF_BLUETOOTH = Bluetooth sockets = Bluetooth apps
            = AF_UNSPEC = Unspecified — used for lookups (not for creating actual sockets)
    // Defines how data is sent (reliable stream, datagrams, raw packets...).
    type = SOCK_STREAM => TCP || SOCK_DGRAM => UDP
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

int sockfd = socket(AF_INET, SOCK_STREAM, 0);
*//////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
/* 		STRUCT SOCKADDR_IN

struct sockaddr_in {
    sa_family_t    sin_family; // Address family (AF_INET)
    uint16_t       sin_port;   // Port number (8080)
    struct in_addr sin_addr;   // IP address (struct with uint32_t s_addr)
    char           sin_zero[8]; // Padding, usually set to 0
};

sin_family → must be same type as socket()
sin_port → port number in network byte order → use htons() to convert
sin_addr → IP address (struct in_addr) → usually set via inet_addr() or INADDR_ANY
	INADDR_ANY → listen on all IP addresses of the machine (== 0.0.0.0)
	or specify a specific IP: server_addr.sin_addr.s_addr = inet_addr("192.168.1.100");
sin_zero → padding to make sockaddr_in same size as sockaddr; usually memset to 0

htons() → Host to Network Short (converts 16-bit port to big-endian)
	in socket programming, if you do something like:
		addr.sin_port = htons(0);
	it tells the kernel:
		“pick any available ephemeral port for me.”
	then, after you bind(), you can find out which port was actually chosen with:
		socklen_t len = sizeof(addr);
		getsockname(sock_fd, (struct sockaddr*)&addr, &len);
		std::cout << "bound port: " << ntohs(addr.sin_port) << std::endl;

	so in short:
		port == 0 → kernel automatically assigns a free port.
		it’s valid, but not what you usually want for a web server, since clients won’t know which port to connect to.

*//////////////////////////////////////////////////////////////////////////////]


///////////////////////////////////////////////////////////////////////////////]
/* 		SETSOCKOPT

    Configures socket options (e.g., SO_REUSEADDR).

int setsockopt(
    int sockfd,					// your socket file descriptor
    int level,					// which layer (level) of options we’re touching
    int option_name,			// the specific option you want to set
    const void *option_value,	// pointer to the value you’re setting
    socklen_t option_len		// size (in bytes) of that value
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
*//////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
/* 		FCNTL

int fcntl(int fd, int cmd, ... args );

ex:
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK); // set non-blocking mode

    Controls file descriptor flags.
    Allowed flags: F_SETFL, O_NONBLOCK, FD_CLOEXEC.
        F_GETFL → get file status flags
        F_SETFL → set file status flags
    Also used for duplication, locking, etc.

*//////////////////////////////////////////////////////////////////////////////]
