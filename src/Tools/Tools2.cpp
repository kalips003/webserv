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
/** Change epoll monitoring for a FD, storing the FD itself.
 * @param epoll_fd  epoll instance FD
 * @param client_fd FD to operate on
 * @param new_flag  event flags (EPOLLIN, EPOLLOUT, EPOLLET...)
 * @param mode      epoll_ctl operation (EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL)
 * @return true on success, false on error		---*/
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

/** Change epoll monitoring for a FD, storing a pointer.
 * @param epoll_fd  epoll instance FD
 * @param client_fd FD to operate on
 * @param ptr       user pointer (e.g., Connection*, Task*)
 * @param new_flag  event flags (EPOLLIN, EPOLLOUT, EPOLLET...) (0 for delete)
 * @param mode      epoll_ctl operation (EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL)
 * @return true on success, false on error		---*/
bool	epollChangeFlags(int epoll_fd, int client_fd, void* ptr, uint32_t new_flag, int mode) {

	struct epoll_event ev;             // for adding/modifying FDs
	ev.events = new_flag;      // Readable, edge-triggered
	ev.data.ptr = ptr;

	if (epoll_ctl(epoll_fd, mode, client_fd, &ev)) {
		printErr("epoll_ctl() <!> WARNING _clients");
		return false;
	}
	return true;
}

#include "defines.hpp"
#include <sys/stat.h>
#include <unistd.h>
///////////////////////////////////////////////////////////////////////////////]
bool dirExists(const char* path) {
	struct stat st;
	return (stat(path, &st) == 0 && (st.st_mode & S_IFDIR));
}
///////////////////////////////////////////////////////////////////////////////]
bool createDir(const char* path, mode_t mode = 0777) {
	if (mkdir(path, mode) != 0) {
		printErr("mkdir()");
		return false;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////]
/** Create a unique temporary file.
 *
 * @param to_store_path_name  Reference to a string that will be filled with the full path
 *                            of the created temporary file.
 * @param root_path           Optional pointer to a string specifying the directory where
 *                            the temp file should be created. If null, defaults to "/tmp".
 *
 * @return File descriptor of the created temp file on success, or -1 on error.
 *
 * Notes:
 * - If the specified root directory does not exist, the function will attempt to create it.
 * - The temp file is created with O_WRONLY | O_CREAT | O_EXCL and permissions 0666.
 * - Attempts up to 1000 unique filenames before failing.		---*/
int	createTempFile(std::string& to_store_path_name, const std::string* root_path) {

	std::string root = "/tmp";
	if (root_path)
		root = *root_path;

	if (!dirExists(root.c_str())) {
		if (!createDir(root.c_str())) {
			oss msg; msg << "Cannot create temp directory: " << root << ". Check permissions!";
			printLog(ERROR, msg.str(), 1);
			return -1;
		}
	}

	std::string file_begin = root + '/' + "webserv_tmp_";
	std::string temp_name;
	int fd = -1;

	oss num;
	for (int i = 0; i < 1000; i++) {

		num.str(""); num.clear();
		num << i;
		temp_name = file_begin + num.str();

		if (access(temp_name.c_str(), F_OK) != 0) {
			fd = open(temp_name.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
			if (fd < 0) {
				printErr("open()");
				return fd;
			}
			to_store_path_name = temp_name;
			return fd;
		}
	}
	printLog(WARNING, "Too many attempts at creating temp file failed", 1);
	return fd;
}