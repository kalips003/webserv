#include "Ft_Get.hpp"
#include "_colors.h"

#include <unistd.h>

#include "Tools1.hpp"

#include <sys/epoll.h>
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
//  need to store: a temp fd; temp_file_name; fd of child to wait

	int pipefd[2];
	if (pipe(pipefd) < 0) {
		printErr("pipe()");
		return 500;
	}

	pid_t pid = fork();
	if (pid < 0) {
		printErr("fork()");
		return 500;
	}

	if (pid == 0) {
		// ---- child ----
		close(pipefd[0]);          // close read end, child writes only
		dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe
		close(pipefd[1]);          // fd duplicated, safe to close
		// setenv(...)
		setenv("REQUEST_METHOD", "GET", 1);
		setenv("QUERY_STRING", query.c_str(), 1);
		setenv("SCRIPT_FILENAME", ressource.c_str(), 1);
		setenv("REDIRECT_STATUS", "200", 1);

		char* argv[] = {
			const_cast<char*>((*CGI_interpreter_path).c_str()),  // argv[0] = program name
			const_cast<char*>(ressource.c_str()), // argv[1] = script path
			NULL
		};

		execve((*CGI_interpreter_path).c_str(), argv, environ);
		printErr("execve()");
		_exit(1);
	}
// ---- parent ----
	close(pipefd[1]); // close write end — parent only reads

	if (epollChangeFlags(_epoll_fd, pipefd[0], _this_connec, EPOLLIN, ??))
		return 500;
	if (epollChangeFlags(_epoll_fd, _client_fd, EPOLLDELETE, NULL))
		return 500;


int	fd_to_store = createTempFile(std::string& to_store_path_name, const std::string* root_path);
	// store child pid;

	setCGIStatus(CGI_DOING);
	return -1;

	// later: 
/*
	int status;
	pid_t result = waitpid(pid, &status, WNOHANG);
	if (result == 0) {
		// child is still running
	} else if (result == pid) {
		// child exited
		// can check status
	} else if (result == -1) {
		perror("waitpid");
	}
*/
}
