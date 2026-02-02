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
	LOG_LOG("path (" <<  _request.getPath() << ") after sanitizePath: " << sanitized);

// set Method::_location_data* from all the location /blocks
	_location_block = SettingsServer::isLocationKnown(sanitized); // validity checked in request.validateLocationBlock()

// add root to path (- location_path)
	std::string ressource; // ressource asked with absolute path: 'root/ressource - query'
	SettingsServer::getFullPath(ressource, sanitized); // validity checked in request.validateLocationBlock()
	LOG_LOG(_request.getMethod() << " ) Full path of the asked ressource: " << ressource);

// if no trailing / in directory path, redirect
	struct stat ressource_info;
	stat(ressource.c_str(), &ressource_info);
	if (S_ISDIR(ressource_info.st_mode) && sanitized[sanitized.size() - 1] != '/') {
		_answer.setFirstLine(301);
		_answer.addToHeaders("Location", sanitized + "/");
		return Connection::SENDING;
	}

// check "Content-Type" = "multipart/form-data";
	int rtrn = this->treatContentType(ressource, query);
	if (rtrn != -1)
		return rtrn;

// HANDLE FILE
	return handleRessource(ressource, query);
}

///////////////////////////////////////////////////////////////////////////////]
/**	handle either
// 	file not exist
// 	file exist
//		directory ---*/
int Method::handleRessource(std::string& ressource, std::string& query) {

// DOESNT EXIST
	int rtrn;
	struct stat ressource_info;
	rtrn = isFileNOK(ressource, ressource_info);
	if (rtrn)
		return this->howToHandleFileNotExist(ressource, rtrn);

// is FILE
	if (S_ISREG(ressource_info.st_mode)) {
		LOG_DEBUG("is FILE");
		const std::string* CGI_interpreter_path = isCGI(ressource); // ptr to /usr/bin/python3;
		if (CGI_interpreter_path)
			return iniCGI(ressource, query, CGI_interpreter_path);
		else
			return this->handleFileExist(ressource);
	}
// is DIRECTORY 
	else if (S_ISDIR(ressource_info.st_mode)) {
		LOG_DEBUG("is DIRECTORY");
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
		sleep(1);

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
			const_cast<char*>("-u"),                               // unbuffered
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
	if (!epollChangeFlags(_data._epoll_fd, pipefd[0], _data._this_ptr, EPOLLIN | EPOLLRDHUP, EPOLL_CTL_ADD))
		return 500;

	if (!_answer.getFile().createTempFile(&g_settings.getTempRoot())) {
		kill(_cgi_data._child_pid, SIGKILL);
		close(_cgi_data._child_pipe_fd);
		return 500;
	}
	_answer.setStatus(HttpObj::READING_HEADER);
	LOG_HERE("iniCGI finished")
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
				LOG_LOG("CGI child (" << _cgi_data._child_pid << ") ended with status: " << WEXITSTATUS(status))
			else
				LOG_LOG("CGI child (" << _cgi_data._child_pid << ") ended in a bloodbath (signal?)")
		}
		_cgi_data._child_pid = -1;

	// reset Answer & Connection to Sending
		_answer.setStatus(HttpObj::SENDING_HEAD);
		return Connection::SENDING;
	}
	int status;
	int waitpid_status = waitpid(_cgi_data._child_pid, &status, WNOHANG); // non blocking
	if (waitpid_status == -1) {
		LOG_ERROR_SYS("Error waiting for child")
		return 500;
	} else if (waitpid_status > 0) {
	LOG_HERE(3)
	// do one last call to flush
		rtrn = _answer.receive_cgi(_data._buffer, _data._sizeofbuff, _cgi_data._child_pipe_fd);
		LOG_DEBUG("after SECOND call to receive_cgi(), rtrn = " << rtrn)
	// remove pipe from epoll and close it
		epollChangeFlags(_data._epoll_fd, _cgi_data._child_pipe_fd, 0, EPOLL_CTL_DEL);
		close(_cgi_data._child_pipe_fd);
		_cgi_data._child_pipe_fd = -1;
	// reset pid
		if (WIFEXITED(status))
			LOG_LOG("CGI child (" << _cgi_data._child_pid << ") ended with status: " << WEXITSTATUS(status))
		else
			LOG_LOG("CGI child (" << _cgi_data._child_pid << ") ended in a bloodbath (signal?)")
		_cgi_data._child_pid = -1;
	// reset Answer & Connection to Sending
		_answer.setStatus(HttpObj::SENDING_HEAD);
		return Connection::SENDING;
	}
	return Connection::DOING_CGI;
}

// int		Method::exec_cgi() {
// 	LOG_LOG("Inside exec CGI");

// 	int rtrn = _answer.receive_cgi(_data._buffer, _data._sizeofbuff, _cgi_data._child_pipe_fd);

// // handle errors
// 	if (rtrn >= 100) {

// 	// remove pipe from epoll and close it
// 		epollChangeFlags(_data._epoll_fd, _cgi_data._child_pipe_fd, 0, EPOLL_CTL_DEL);
// 		close(_cgi_data._child_pipe_fd);
// 		_cgi_data._child_pipe_fd = -1;
// 	// kill child
// 		kill(_cgi_data._child_pid, SIGKILL); // force stop
// 		waitpid(_cgi_data._child_pid, NULL, 0); // reap fully, blocking ok
// 		_answer.createError(rtrn);
// 		LOG_LOG("CGI child (" << _cgi_data._child_pid << ") got killed after bad rtrn (" << rtrn << ")")

// 		_cgi_data._child_pid = -1;
// 	// reset Answer & Connection to Sending
// 		_answer.setStatus(HttpObj::SENDING_HEAD);
// 		return Connection::SENDING;
// 	}
// 	usleep(5000);
// 	LOG_HERE(1)
// // wait child
// 	int status;
// 	int waitpid_status;
// 	while ((waitpid_status = waitpid(_cgi_data._child_pid, &status, WNOHANG)) == 0) {
// 		rtrn = _answer.receive_cgi(_data._buffer, _data._sizeofbuff, _cgi_data._child_pipe_fd);
// 		LOG_DEBUG("its not working bitch: " << rtrn)
// 	}
// 	LOG_HERE(2 << "waitpid_status: " << waitpid_status)
// 	if (waitpid_status == -1) {
// 		LOG_ERROR_SYS("Error waiting for child")
// 		return 500;
// 	} else if (waitpid_status > 0) {
// 	LOG_HERE(3)
// 	// do one last call to flush
// 		rtrn = _answer.receive_cgi(_data._buffer, _data._sizeofbuff, _cgi_data._child_pipe_fd);
// 		LOG_DEBUG("after SECOND call to receive_cgi(), rtrn = " << rtrn)
// 	// remove pipe from epoll and close it
// 		epollChangeFlags(_data._epoll_fd, _cgi_data._child_pipe_fd, 0, EPOLL_CTL_DEL);
// 		close(_cgi_data._child_pipe_fd);
// 		_cgi_data._child_pipe_fd = -1;
// 	// reset pid
// 		if (WIFEXITED(status))
// 			LOG_LOG("CGI child (" << _cgi_data._child_pid << ") ended with status: " << WEXITSTATUS(status))
// 		else
// 			LOG_LOG("CGI child (" << _cgi_data._child_pid << ") ended in a bloodbath (signal?)")
// 		_cgi_data._child_pid = -1;
// 	// reset Answer & Connection to Sending
// 		_answer.setStatus(HttpObj::SENDING_HEAD);
// 		return Connection::SENDING;
// 	}

// // child still running
// 	return Connection::DOING_CGI;
// }



int		Method::exec_cgi() {

	// function to read from pipe and fill answer
	int rtrn = _answer.receive_cgi(_data._buffer, _data._sizeofbuff, _cgi_data._child_pipe_fd);
	// should return 0 on eof (but eof never read because Method::exec_cgi() is only 
	// 		called when epoll wake me up (valid data in pipe))

	if (rtrn >= 100) {
		// handle errors
	}



// give time to kernel to end the child 
	usleep(5000);

	int status;
	int waitpid_status;
	waitpid_status = waitpid(_cgi_data._child_pid, &status, WNOHANG) {
		rtrn = _answer.receive_cgi(_data._buffer, _data._sizeofbuff, _cgi_data._child_pipe_fd);
		LOG_DEBUG("its not working bitch: " << rtrn)
	}
	LOG_HERE(2 << "waitpid_status: " << waitpid_status)
	if (waitpid_status == -1) {
		LOG_ERROR_SYS("Error waiting for child")
		return 500;
	if (waitpid_status > 0) {

	// do one last call to flush and detect correctly EOF
		rtrn = _answer.receive_cgi(_data._buffer, _data._sizeofbuff, _cgi_data._child_pipe_fd);
		LOG_DEBUG("after SECOND call to receive_cgi(), rtrn = " << rtrn)
	// remove pipe from epoll and close it
		epollChangeFlags(_data._epoll_fd, _cgi_data._child_pipe_fd, 0, EPOLL_CTL_DEL);
		close(_cgi_data._child_pipe_fd);
		_cgi_data._child_pipe_fd = -1;
	// reset pid
		if (WIFEXITED(status))
			LOG_LOG("CGI child (" << _cgi_data._child_pid << ") ended with status: " << WEXITSTATUS(status))
		else
			LOG_LOG("CGI child (" << _cgi_data._child_pid << ") ended in a bloodbath (signal?)")
		_cgi_data._child_pid = -1;
	// reset Answer & Connection to Sending
		_answer.setStatus(HttpObj::SENDING_HEAD);
		return Connection::SENDING;
	}

// child still running
	return Connection::DOING_CGI;
}