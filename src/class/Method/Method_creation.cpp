#include "Method.hpp"
#include "Log.hpp"

#include "Connection.hpp"

#include <signal.h>
#include "Tools2.hpp"
#include <sys/wait.h>
#include <sys/epoll.h> 

#include "Ft_Get.hpp"
#include "Ft_Post.hpp"
#include "Ft_Put.hpp"
#include "Ft_Delete.hpp"
#include "Ft_Head.hpp"
#include "Ft_Options.hpp"

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
Method::~Method() {

// _tmp_file handle its destuction

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
Method*		Method::createTask(const std::string& method, const t_connec_data& data) {
	Method::Ft_Type type = Method::parseMethod(method);

	// LOG_LOG("createTask().data path: " << data._this_ptr->getRequest().getPath());
	switch(type) {
		case GET:		return new Ft_Get(data);
		case POST:		return new Ft_Post(data);
		case DELETE:	return new Ft_Delete(data);
		case HEAD:		return new Ft_Head(data);
		case PUT:		return new Ft_Put(data);
		case OPTIONS:	return new Ft_Options(data);
		default:		return NULL;  // unknown method → reject (405)
	}
}

///////////////////////////////////////////////////////////////////////////////]
Method::Ft_Type		Method::parseMethod(const std::string& method) {
	if (method == "GET") return GET;
	if (method == "POST") return POST;
	if (method == "DELETE") return DELETE;
	if (method == "PUT") return PUT;
	if (method == "HEAD") return HEAD;
	if (method == "OPTIONS") return OPTIONS;
	return UNKNOWN;
}