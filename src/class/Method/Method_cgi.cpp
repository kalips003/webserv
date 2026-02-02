#include "Method.hpp"

#include <signal.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "Tools2.hpp"

extern char **environ;
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
int Method::iniCGI(const std::string& ressource, const std::string& query, const std::string* CGI_interpreter_path) {

	int pipefd[2];
	if (pipe(pipefd) < 0) {
		LOG_ERROR_SYS("Method::iniCGI(): pipe()");
		return 500;
	}

	pid_t pid = fork();
	if (pid < 0) {
		LOG_ERROR_SYS("Method::iniCGI(): fork()");
		return 500;
	}

	if (pid == 0) {
//-----------------------------------------------------------------------------]
// ---- child ----
		dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe

		//	close all unused fd by the child (including pipefd[0] + pipefd[1])
		int max_fd = sysconf(_SC_OPEN_MAX);
		int fd_body_tmp = _request.getBodyFd();
		for (int i = 0; i < max_fd; ++i) {
			if (i == STDIN_FILENO || i == STDOUT_FILENO || i == STDERR_FILENO || i == fd_body_tmp)
				continue;
			close(i);
		}

		this->prepareChild(ressource, query);

		char* argv[] = {
			const_cast<char*>((*CGI_interpreter_path).c_str()),  // argv[0] = program name
			const_cast<char*>(ressource.c_str()), // argv[1] = script path
			NULL
		};
		execve((*CGI_interpreter_path).c_str(), argv, environ);
		LOG_ERROR_SYS(RED "Method::iniCGI( CHILD ): execve()" RESET);
		_exit(1);
	}
//-----------------------------------------------------------------------------]
// ---- parent ----
	close(pipefd[1]); // close write end — parent only reads
	fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

	_cgi_data._child_pipe_fd = pipefd[0];
	_cgi_data._child_pid = pid;

	if (!epollChangeFlags(_data._epoll_fd, _data._client_fd, 0, EPOLL_CTL_DEL))
		return 500;
	if (!epollChangeFlags(_data._epoll_fd, pipefd[0], _data._this_ptr, EPOLLIN | (EPOLLRDHUP | EPOLLHUP), EPOLL_CTL_ADD))
		return 500;

	if (!_answer.getFile().createTempFile(&g_settings.getTempRoot())) {
		kill(_cgi_data._child_pid, SIGKILL);
		close(_cgi_data._child_pipe_fd);
		return 500;
	}
	_answer.setStatus(HttpObj::READING_HEADER);
	return Connection::DOING_CGI;
}

#include <sys/wait.h>
///////////////////////////////////////////////////////////////////////////////]
/**	Funciton called on second loop, once _cgi_status == CGI_DOING, 
* child is set-up and so on 
* @return -1 if cgi still going 
* @return Connection::SENDING if cgi finished (and handled)
* @return ErrCode in the case of any error
*/
int		Method::exec_cgi() {
	LOG_LOG("Inside exec CGI");

	int rtrn = _answer.receive_cgi(_data._buffer, _data._sizeofbuff, _cgi_data._child_pipe_fd);
	if (rtrn >= 100 || static_cast<HttpObj::HttpBodyStatus>(rtrn) == HttpObj::CLOSED) {

	// remove pipe from epoll and close it
		epollChangeFlags(_data._epoll_fd, _cgi_data._child_pipe_fd, 0, EPOLL_CTL_DEL);
		close(_cgi_data._child_pipe_fd);
		_cgi_data._child_pipe_fd = -1;

	// wait child
		if (rtrn >= 100) { // error quit
			kill(_cgi_data._child_pid, SIGKILL); // force stop
			waitpid(_cgi_data._child_pid, NULL, 0); // reap fully, blocking ok
			_answer.createError(rtrn);
		} else { // EOF
			int status;
			waitpid(_cgi_data._child_pid, &status, WNOHANG); // non blocking
			if (WIFEXITED(status))
				LOG_LOG("CGI child (pid=" C_431 << _cgi_data._child_pid << RESET ") ended with status: " << WEXITSTATUS(status))
			else
				LOG_LOG("CGI child (pid=" C_431 << _cgi_data._child_pid << RESET ") ended in a bloodbath (signal?)")
		}
		_cgi_data._child_pid = -1;

	// reset Answer & Connection to Sending
		_answer.setBytesWritten(0);
		_answer.getFile().resetFileFdBegining();
		_answer.setStatus(HttpObj::SENDING_HEAD);
		return Connection::SENDING;
	}
	LOG_HERE("exec_cgi(), exiting: status = " << rtrn)
	return Connection::DOING_CGI;
}
