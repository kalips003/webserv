#include <fcntl.h>
#include "Tools1.hpp"
#include "_colors.h"

///////////////////////////////////////////////////////////////////////////////]
//	int     fcntl(int fd, int cmd, ... /* arg */ );
// CMDs:
// F_GETFL = get current file status flags (O_NONBLOCK, O_APPEND, ...).
// F_SETFL = set flags with 'arg'
// 
// FLAGS:
// O_NONBLOCK	make reads/writes non-blocking
// O_APPEND		always write at the end of file
// O_SYNC		write operations wait for physical completion
// O_ASYNC		enable SIGIO delivery when I/O is possible (less used)
///////////////////////////////////////////////////////////////////////////////]
/**
 * Set flags for the given fd

 * @param fd_to_set   FD to set flags to
 * @param flag_to_add   OR sum of the flags to add
 * @return         FALSE on errors, TRUE otherwise
 */
bool	set_flags(int fd_to_set, int flag_to_add) {

	int flags = fcntl(fd_to_set, F_GETFL, 0);
	if (flags < 0){
		printErr(RED "fcntl()" RESET);
		return false;
	}
	if (fcntl(fd_to_set, F_SETFL, flags | flag_to_add) < 0){
		printErr(RED "accept()" RESET);
		return false;
	}
	return true;
}

#include <sys/epoll.h>
///////////////////////////////////////////////////////////////////////////////]
    // ACCEPT_OK = 1,        // one client accepted
    // ACCEPT_EMPTY = 0,     // no more clients (EAGAIN)
    // ACCEPT_RETRY = -1,    // EINTR / ECONNABORTED
    // ACCEPT_FATAL = -2   
///////////////////////////////////////////////////////////////////////////////]
bool	epollChangeFlags(int epoll_fd, int client_fd, uint32_t new_flag, int mode) {

	struct epoll_event ev;             // for adding/modifying FDs
	ev.events = new_flag;      // Readable, edge-triggered
	ev.data.fd = client_fd;

	if (epoll_ctl(epoll_fd, mode, client_fd, &ev)) {
		printErr("epoll_ctl() <!> WARNING _clients");
		return false;
	}
	return true;
}
