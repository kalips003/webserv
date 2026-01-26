#include "Task.hpp"

#include "Connection.hpp"

#include <signal.h>
#include "Tools2.hpp"
#include <sys/wait.h>
#include <sys/epoll.h> 

#include "Ft_Get.hpp"
#include "Ft_Post.hpp"
#include "Ft_Delete.hpp"
///////////////////////////////////////////////////////////////////////////////]
Task::Task(Connection& connec, int epoll)
 : _request(connec.getRequest()), _answer(connec.getAnswer()), _cgi_status(CGI_NONE), _cgi_data(), _location_block(NULL) {
	_data._client_fd = connec.getClientFd();
	_data._epoll_fd = epoll;
	_data._this_ptr = &connec;
}

///////////////////////////////////////////////////////////////////////////////]
/*
Set up a SIGCHLD handler:

signal(SIGCHLD, [](int){ 
    while (waitpid(-1, nullptr, WNOHANG) > 0) { reap all exited children }});

-1 → wait for any child
WNOHANG → don’t block if no child has exited
Loops because multiple children may exit at once
Now, when you want to terminate a CGI child:

kill(child_pid, SIGKILL);

You don’t need to call waitpid() manually
*/
// #include <signal.h>
// #include "Tools2.hpp"
// #include <sys/wait.h>
// #include <sys/epoll.h> 
///////////////////////////////////////////////////////////////////////////////]
/**
// what Task owns:
	_cgi_data._child_pid;
	_cgi_data._child_pipe_fd;
	_cgi_data._tmp_file_fd;
	_cgi_data._tmp_file_name;
 */
Task::~Task() {

	if (!_cgi_data._tmp_file_name.empty()) {
		unlink(_cgi_data._tmp_file_name.c_str()); // be sure that we have WX permission on the default temp folder
		_cgi_data._tmp_file_name.clear();
	}
	if (_cgi_data._tmp_file_fd >= 0) {
		close(_cgi_data._tmp_file_fd);
		_cgi_data._tmp_file_fd = -1;
	}
	if (_cgi_data._child_pipe_fd >= 0) {
		epollChangeFlags(_data._epoll_fd, _cgi_data._child_pipe_fd, 0, EPOLL_CTL_DEL);
		close(_cgi_data._child_pipe_fd);
		_cgi_data._child_pipe_fd = -1;
	}
	if (_cgi_data._child_pid >= 0) {
		kill(_cgi_data._child_pid, SIGKILL);
		int status;
		waitpid(_cgi_data._child_pid, &status, 0); // blocking reap
		_cgi_data._child_pid = -1;
	}
}


// #include "Ft_Get.hpp"
// #include "Ft_Post.hpp"
// #include "Ft_Delete.hpp"
///////////////////////////////////////////////////////////////////////////////]
Task* Task::createTask(const std::string& method, Connection& connec, int epoll_fd) {
	if (method == "GET")
		return new Ft_Get(connec, epoll_fd);
	else if (method == "POST")
		return new Ft_Post(connec, epoll_fd);
	else if (method == "DELETE")
		return new Ft_Delete(connec, epoll_fd);
	// ... other methods
	else
		return NULL;  // unknown method → 405 or reject
}
