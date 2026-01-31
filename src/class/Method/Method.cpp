#include "Method.hpp"
#include "Log.hpp"

#include "Tools2.hpp"

#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <iostream>
#include <algorithm>

#include "SettingsServer.hpp"
#include "HttpRequest.hpp"
#include "HttpAnswer.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** Common handling of a request for normal (non-CGI) processing.
 * - Extracts query string from the path (/script.py - ?x=abc&y=42)
 * - Resolves full filesystem path (/root/_path)
 * - Checks existence and type of the resource
 * - Dispatches to Derived Class the handling of CGI, file, or directory as appropriate
 *
 * @return Connection::SENDING on success, or HTTP error code on failure	---*/
int Method::normal_doing() {

// is there query in the path? > /script.py?x=abc&y=42
	std::string path;
	std::string query;
	size_t pos = _request.getPath().find_first_of('?');
	if (pos == std::string::npos)
		path = _request.getPath();
	else {
		path = _request.getPath().substr(0, pos);
		query = _request.getPath().substr(pos + 1);
	}
	
// sanitize given_path for %XX;
	std::string sanitized;
	if (SettingsServer::sanitizePath(sanitized, path))
		return 400;
	LOG_LOG("path after sanitizePath: " << sanitized);

// set Method::_location_data* from all the location /blocks
	_location_block = SettingsServer::isLocationKnown(sanitized); // validity checked in request.validateLocationBlock()

// add root to path (- location_path)
	std::string ressource; // ressource asked with absolute path: 'root/ressource - query'
	SettingsServer::getFullPath(ressource, sanitized); // validity checked in request.validateLocationBlock()
	LOG_LOG("Full path of the asked ressource: " << ressource);

// HANDLE FILE
	return handleFile(ressource, query, sanitized);
}

///////////////////////////////////////////////////////////////////////////////]
int Method::handleFile(std::string& ressource, std::string& query, std::string& sanitized) {

// DOESNT EXIST
	int rtrn;
	struct stat ressource_info;
	rtrn = isFileNOK(ressource, ressource_info);
	if (rtrn)
		return this->howToHandleFileNotExist(ressource, rtrn);

// is FILE
	if (S_ISREG(ressource_info.st_mode)) {
		LOG_LOG("is FILE");
		const std::string* CGI_interpreter_path = isCGI(ressource); // ptr to /usr/bin/python3;
		if (CGI_interpreter_path)
			return iniCGI(ressource, query, CGI_interpreter_path);
		else
			return this->handleFileExist(ressource);
	}
// is DIRECTORY 
	else if (S_ISDIR(ressource_info.st_mode)) {
		LOG_LOG("is DIRECTORY");
		if (sanitized[sanitized.size() - 1] != '/') { // if no trailing / in directory path, redirect
			_answer.setFirstLine(301);
			_answer.addToHeaders("Location", sanitized + "/");
			return Connection::SENDING;
		}
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
		LOG_ERROR_SYS(RED "Method::iniCGI( CHILD ): execve()" RESET);
		_exit(1);
	}
//-----------------------------------------------------------------------------]
// ---- parent ----
	close(pipefd[1]); // close write end — parent only reads
	fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

	cgi_data&	cgi_data = getCGIData();
	cgi_data._child_pipe_fd = pipefd[0];
	cgi_data._child_pid = pid;

	if (epollChangeFlags(getData()._epoll_fd, getData()._client_fd, 0, EPOLL_CTL_DEL))
		return 500;
	if (epollChangeFlags(getData()._epoll_fd, pipefd[0], getData()._this_ptr, EPOLLIN | EPOLLRDHUP, EPOLL_CTL_ADD))
		return 500;

	if (!cgi_data._tmp_file.createTempFile(&g_settings.getTempRoot())) {
		kill(cgi_data._child_pid, SIGKILL);
		close(cgi_data._child_pipe_fd);
		return 500;
	}

	return Connection::DOING_CGI;

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
