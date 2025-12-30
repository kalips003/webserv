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
		printErr(RED "fcntl( F_GETFL ) failed" RESET);
		return false;
	}
	if (fcntl(fd_to_set, F_SETFL, flags | flag_to_add) < 0){
		printErr(RED "accept( F_SETFL ) failed" RESET);
		return false;
	}
	return true;
}