#include "Task.hpp"

#include "HttpRequest.hpp"

#include <iostream>

#include "Tools1.hpp"

#include <sys/stat.h>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "SettingsServer.hpp"
#include "Tools2.hpp"

///////////////////////////////////////////////////////////////////////////////]
int Task::ft_do() {


	this->printHello();

	if (_cgi_status == CGI_DOING)
		return this->exec_cgi();
	else
		return normal_doing();
}

#include <algorithm>
///////////////////////////////////////////////////////////////////////////////]
/** Common handling of a request for normal (non-CGI) processing.
 * - Extracts query string from the path (/script.py - ?x=abc&y=42)
 * - Resolves full filesystem path (/root/_path)
 * - Checks existence and type of the resource
 * - Dispatches to Derived Class the handling of CGI, file, or directory as appropriate
 *
 * @return 0 on success, or HTTP error code on failure	---*/
int Task::normal_doing() {

// is there query in the path? > /script.py?x=abc&y=42
	const HttpRequest& req = getRequest();
	size_t pos = req.getPath().find_first_of('?');
	std::string path;
	std::string query;
	if (pos == std::string::npos)
		path = req.getPath();
	else {
		path = req.getPath().substr(0, pos);
		query = req.getPath().substr(pos + 1);
	}

// sanitize given_path for %XX;
	std::string sanitized;
	if (sanitizePath(sanitized, path))
		return 400;
// set Task::_location_data* from all the location /blocks
	if (!(_location_block = isLocationKnown(sanitized)))
		return 500;
// Once Location block is known, check if method is allowed
	if (std::find(_location_block->data.allowed_methods.begin(), _location_block->data.allowed_methods.end(), getRequest().getMethod()) == _location_block->data.allowed_methods.end())
		return 405;
		
// add root to path (- location_path)
	std::string ressource; // ressource asked with full path: 'root/ressource - query'
	int rtrn = getFullPath(ressource, sanitized);
	if (rtrn)
		return rtrn;
	oss msg; msg << "Full path of the asked ressource: " << ressource;
	printLog(DEBUG, msg.str(), 1);

// check existance
	struct stat ressource_info;
	rtrn = isFileNOK(ressource, ressource_info);
	if (rtrn)
		return this->howToHandleFileNotExist(ressource, rtrn);

// is FILE
	if (S_ISREG(ressource_info.st_mode)) {
		printLog(DEBUG, "is FILE", 1);
		const std::string* CGI_interpreter_path = isCGI(ressource); // ptr to /usr/bin/python3;
		if (CGI_interpreter_path)
			return iniCGI(ressource, query, CGI_interpreter_path);
		else
			return this->handleFile(ressource, ressource_info);
	}
// is DIRECTORY
	else if (S_ISDIR(ressource_info.st_mode)) {
		printLog(DEBUG, "is DIRECTORY", 1);
		return this->handleDir(ressource);
	}
	else
		return 403; // other filesystem objects: symlinks, sockets, devices, FIFOs…
}

// #include <signal.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/epoll.h>
// #include "SettingsServer.hpp"
// #include "Tools2.hpp"
#include <unistd.h>
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
int Task::iniCGI(const std::string& ressource, const std::string& query, const std::string* CGI_interpreter_path) {
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
		dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe

//	close all unused fd by the child
		// close(pipefd[0]);          // close read end, child writes only
		// close(pipefd[1]);          // fd duplicated, safe to close
		int max_fd = sysconf(_SC_OPEN_MAX);
		int fd_body_tmp = getRequest().getFdBody();
		for (int i = 0; i < max_fd; ++i) {
			if (i == STDIN_FILENO || i == STDOUT_FILENO || i == fd_body_tmp)
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
		printErr("execve()");
		_exit(1);
	}
// ---- parent ----
	close(pipefd[1]); // close write end — parent only reads
	fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

	cgi_data&	cgi_data = getCGIData();
	cgi_data._child_pipe_fd = pipefd[0];
	cgi_data._child_pid = pid;

	if (epollChangeFlags(getData()._epoll_fd, pipefd[0], getData()._this_ptr, EPOLLIN | EPOLLRDHUP, EPOLL_CTL_ADD))
		return 500;
	if (epollChangeFlags(getData()._epoll_fd, getData()._client_fd, 0, EPOLL_CTL_DEL))
		return 500;

	cgi_data._tmp_file_fd = createTempFile(cgi_data._tmp_file_name, g_settings.find_setting("tmp_root"));
	if (cgi_data._tmp_file_fd < 0) {
		kill(cgi_data._child_pid, SIGKILL);
		close(cgi_data._child_pipe_fd);
		return 500;
	}

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
