#include "Log.hpp"
#include "Server.hpp"
#include "_colors.h"

#include <iostream>
#include <unistd.h>

#include "Tools1.hpp"
#include "Tools2.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** Constructor for the Server.
 *
 * Takes as argument a VALID char* with the path of the config file for the server.
 *
 * Once the construction is finished, _server_status holds the status of the construction (OK / NOK).
 *
 * If _server_status == OK, the server is listening and ready for accept() / epoll()
 *
 * @param confi_file   Path of config file.
 * @return         _server_status true if parsing succeeded, false otherwise.		---*/
Server::Server( const char* confi_file ) : _epoll_fd(-1), _server_status(false) {

	_server_status = g_settings.parse_config_file(confi_file);
	if (!_server_status)
		return ;

	_server_status = create_all_listening_socket();
	if (!_server_status)
		return ;

	_server_status = create_epoll();
	if (!_server_status)
		return ;

	_server_status = init_signals();
	if (!_server_status)
		return ;

	_server_status = true;
	LOG_INFO(C_151 "Server fully set up, serving " RESET << _sockets.size() << C_151 " domains")
}

// #include <unistd.h>
///////////////////////////////////////////////////////////////////////////////]
Server::~Server( void ) {

	for (map_clients::iterator it = _clients.begin(); it != _clients.end(); )
		it = pop_connec(it, false);
	if (_epoll_fd >= 0) close(_epoll_fd);
	for (std::map<int, server_listen>::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
		if (it->second._socket_fd >= 0) close(it->second._socket_fd);
}

///////////////////////////////////////////////////////////////////////////////]
bool	Server::create_all_listening_socket() {

// std::map<int, server_listen>	_sockets;
	for (std::map<std::string, Settings::server_setting>::iterator it = g_settings._global_blocks.begin();
		it != g_settings._global_blocks.end(); ++it) {
		
		if (it->second._server_block_name == "server" && it->second._port > 0) {
			if (_sockets.find(it->second._port) != _sockets.end()) {
				LOG_WARNING("create_all_listening_socket(): listening port " C_154 << it->second._port << "already in use")
				continue;
			}

			Server::server_listen new_socket(it->second);
			new_socket._listen_port = it->second._port;

			if (!Server::create_listening_socket(new_socket)) {
				LOG_WARNING("create_all_listening_socket(): creation socket for port " C_154 << it->second._port << "failed")
				continue;
			}
			LOG_INFO(C_151 "Server " RESET << new_socket._settings._server_name << C_151 " up and running on port: " RESET << it->second._port)
			_sockets.insert(std::pair<int, server_listen>(new_socket._listen_port, new_socket));
		}	
	}

	return _sockets.size();
}

#include <fcntl.h>
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
/**  Create the listening socket.
 *
 * Fills the struct sockaddr_in _addr
 *
 * If return True, the socket is listening and ready to accept
 *
 * @return         FALSE on any error (and print the err msg), TRUE otherwise	---*/
bool	Server::create_listening_socket(Server::server_listen& new_socket) {

	new_socket._socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (new_socket._socket_fd < 0) {
		LOG_ERROR_SYS(RED "create_listening_socket(" RESET << new_socket._listen_port << RED "): socket() failed" RESET)
		return false;
	}
	new_socket._addr.sin_port = htons(new_socket._listen_port);
	new_socket._addr.sin_family = AF_INET;
	new_socket._addr.sin_addr.s_addr = INADDR_ANY;

// in case of rapid on/off of the server (TIME_WAIT state), avoid the EADDRINUSE bind error
	int opt = 1;
	setsockopt(new_socket._socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	int bind_status = bind(new_socket._socket_fd, (struct sockaddr *)&new_socket._addr, sizeof(new_socket._addr));
	if (bind_status) {
		LOG_ERROR_SYS(RED "create_listening_socket(" RESET << new_socket._listen_port << RED "): bind() failed" RESET)
		close(new_socket._socket_fd);
		return false;
	}

	int listen_status = listen(new_socket._socket_fd, HOW_MANY_REQUEST_PER_LISTEN);
// 3 step handshake: SYN > SYN-ACK > ACK validated
	if (listen_status) {
		LOG_ERROR_SYS(RED "create_listening_socket(" RESET << new_socket._listen_port << RED "): listen() failed" RESET)
		close(new_socket._socket_fd);
		return false;
	}

	if (!set_flags(new_socket._socket_fd, O_NONBLOCK)) {
		close(new_socket._socket_fd);
		return false;
	}
	return true;
}

#include <sys/epoll.h>
///////////////////////////////////////////////////////////////////////////////]
bool	Server::create_epoll() {

	_epoll_fd = epoll_create1(0); // manager return an fd of the epoll instance

	// EPOLL_CLOEXEC = Set the close-on-exec (FD_CLOEXEC) flag on the new fd. Prevents FD from leaking to child processes after exec()
	if (_epoll_fd == -1) {
		LOG_ERROR_SYS(RED "epoll_create1()" RESET);
		return false;
	}

	for (std::map<int, server_listen>::iterator it = _sockets.begin(); it != _sockets.end(); ++it) {
	
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.ptr = &it->second;
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, it->second._socket_fd, &ev)) {
			LOG_ERROR_SYS(RED "create_epoll(): epoll_ctl(" RESET << it->second._socket_fd << RED ")" RESET);
			return false;
		}
	}

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
