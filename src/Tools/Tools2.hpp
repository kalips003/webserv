#ifndef TOOLS2_HPP
#define TOOLS2_HPP

///////////////////////////////////////////////////////////////////////////////]
/**
 * Set flags for the given fd

 * @param fd_to_set   FD to set flags to
 * @param flag_to_add   OR sum of the flags to add
 * @return         FALSE on errors, TRUE otherwise
 */
bool	set_flags(int fd_to_set, int flag_to_add);

///////////////////////////////////////////////////////////////////////////////]
/** vafanculo */
bool	epollChangeFlags(int epoll_fd, int client_fd, uint32_t new_flag, int mode);


#endif