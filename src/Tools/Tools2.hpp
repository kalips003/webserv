#ifndef TOOLS2_HPP
#define TOOLS2_HPP

#include <stdint.h>
#include <sys/types.h>
///////////////////////////////////////////////////////////////////////////////]
/**
 * Set flags for the given fd

 * @param fd_to_set   FD to set flags to
 * @param flag_to_add   OR sum of the flags to add
 * @return         FALSE on errors, TRUE otherwise
 */
bool	set_flags(int fd_to_set, int flag_to_add);

///////////////////////////////////////////////////////////////////////////////]
/** change flags for epoll */
bool	epollChangeFlags(int epoll_fd, int client_fd, uint32_t new_flag, int mode);
bool	epollChangeFlags(int epoll_fd, int client_fd, void* ptr, uint32_t new_flag, int mode);

///////////////////////////////////////////////////////////////////////////////]
bool	dirExists(const char* path);
bool	createDir(const char* path, mode_t mode = 0777);
int		createTempFile(std::string& to_store_path_name, const std::string* root_path);

#endif