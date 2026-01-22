#include "Ft_Get.hpp"
#include "_colors.h"

#include <unistd.h>

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** Initialize CGI execution.
 *
 * - fork the CGI process
 * - create and register the CGI pipe in epoll
 * - unregister the connection socket from epoll
 * - create a temporary file to store CGI output
 * - set internal state to CGI_DOING
 *
 * @param ressource Full filesystem path of the requested script
 * @param query     Query string (empty if none)
 * @param CGI_interpreter_path Path to the CGI interpreter (e.g. /usr/bin/python3)
 *
 * @return -1 on successful CGI initialization (processing continues asynchronously),
 *         HTTP error code (>0) on failure			---*/
int Ft_Get::iniCGI(const std::string& ressource, const std::string& query, const std::string* CGI_interpreter_path) {
// need access to connection ptr, connection fd
//  need a temp fd stored somewere



	pid_t pid = fork();
	if (pid < 0) {
		printErr("fork()");
		return 500;
	}

	if (pid == 0) {
		// ---- child ----
		// setenv(...)
		// dup2(...) stdin/stdout
		// execve(...)
		_exit(1); // only if execve fails
	
	}

// ---- parent ----
// read from pipe
// waitpid(pid, ...)



	return 0;
}
