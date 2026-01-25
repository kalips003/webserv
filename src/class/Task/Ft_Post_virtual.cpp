#include "Ft_Post.hpp"

#include "Tools1.hpp"
#include "Tools2.hpp"
#include "HttpAnswer.hpp"
#include "HttpRequest.hpp"

///////////////////////////////////////////////////////////////////////////////]
void Ft_Post::printHello() {
	printLog(DEBUG, "Post method called", 1);
}

///////////////////////////////////////////////////////////////////////////////]
/**	Funciton called on second loop, once _cgi_status == CGI_DOING, 
* child is set-up and so on 
* @return -1 if cgi still going 
* @return 0 if cgi finished (and handled)
* @return ErrCode in the case of any error	---*/
int		Ft_Post::exec_cgi() {
	printLog(ERROR, "--> you have to do this part (execcgi)", 1);

//
	// Implementation
	// return -1;
//



// if implementation finished:
	// setCGIStatus() = CGI_NONE;

	// int code_rtrn_child;
	// waitpid(_cgi_data._child_pid, &code_rtrn_child, 0);
	// _cgi_data._child_pid = -1;

	// close(_cgi_data._child_pipe_fd);
	// _cgi_data._child_pipe_fd = -1;

	// epollChangeFlags(_data._epoll_fd, _data._client_fd, _data._this_ptr, EPOLL_CTL_ADD);
	// return errCode;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////]
/**	rtrn_open == 404 / 403
* In Post, reject file existance?  */
// POST /images/file
// > path/to/root/file
int		Ft_Post::howToHandleFileNotExist(const std::string& ressource, int rtrn_open) {

// does folder we wanna put the file in exist? and permission?
	std::string folder_path = ressource.substr(0, ressource.find_last_of("/"));

	if (folder_path.empty()) // only happen if location_root == '/'
		folder_path = "/"; // delete a file in "/"

	if (access(folder_path.c_str(), W_OK | X_OK) != 0) // need write / exec permissions
		return 403;
//	
	if (rtrn_open == 404) {

		const std::string tmp_path = getRequest().getBodyPath();
		if (tmp_path.empty())
			return 400; // the request didnt include a body

		if (rename(tmp_path.c_str(), ressource.c_str()) < 0) {
			printErr("rename()");
			return 500;
		}
		// HttpRequest still own the fd and /path of the temp, but cleared in destructor
		getAnswer().setFirstLine(201);
		return 0;
	}
	else {// if (rtrn_open == 403)
		printLog(WARNING, "Post request File already exist: permission error", 1);
		return 403;
	}
}

///////////////////////////////////////////////////////////////////////////////]
int		Ft_Post::handleFile(std::string& path, struct stat& ressource_info) {
	(void)ressource_info;

// does folder we wanna put the file in exist? and permission?
	std::string folder_path = path.substr(0, path.find_last_of("/"));

	if (folder_path.empty()) // only happen if location_root == '/'
		folder_path = "/"; // delete a file in "/"

	if (access(folder_path.c_str(), W_OK | X_OK) != 0) // need write / exec permissions
		return 403;
//	
	if (access(path.c_str(), W_OK) != 0) // even if file exist, might not be readable by server
		return 403;

//	
	std::string post_policy = getLocationBlock()->data.post_policy;
	if (post_policy == "reject") {
		printLog(WARNING, "Post request File already exist: post_policy = reject", 1);
		return 409;
	}
	else if (post_policy == "replace") { // open(O_WRONLY | O_CREAT | O_TRUNC)
		const std::string tmp_path = getRequest().getBodyPath();
		if (rename(tmp_path.c_str(), path.c_str()) < 0) { // if file already exist, replace it silently
			printErr("rename()");
			return 500;
		}
	}
	else if (post_policy == "append") {// open(O_WRONLY | O_CREAT | O_APPEND)

		const std::string tmp_path = getRequest().getBodyPath();
		std::ifstream src(tmp_path.c_str(), std::ios::binary);
		std::ofstream dest(path.c_str(), std::ios::binary | std::ios::app);
		if (!src.is_open() || !dest.is_open())
    		return 500;
		dest << src.rdbuf();
		if (dest.fail() || src.fail()) {
			oss msg; msg << "Append from: " << tmp_path << " to: " << path << " failed\n";
			printErr((msg.str()).c_str());
    		return 500; // partial write happened
		}
	}
	else {
		printLog(WARNING, "Post request File already exist: post_policy = unknown", 1);
		return 500;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////]
/** */
int		Ft_Post::handleDir(std::string& ressource) {
	(void)ressource;
	return 403; // or 405 with header {Allow: GET, HEAD}
}

#include <cstdlib> 
#include "HttpRequest.hpp"
///////////////////////////////////////////////////////////////////////////////]
/** */
void	Ft_Post::prepareChild(const std::string& ressource, const std::string& query) {

	setenv("REQUEST_METHOD", "POST", 1);
	setenv("QUERY_STRING", query.c_str(), 1);
	setenv("SCRIPT_FILENAME", ressource.c_str(), 1);
	setenv("REDIRECT_STATUS", "200", 1);

	std::string content_length = itostr(getRequest().getBodySize());
	setenv("CONTENT_LENGTH", content_length.c_str(), 1);
	std::string content_type = getRequest().find_setting("Content-Type");
	if (content_type.empty())
		printLog(WARNING, "CGI POST, Content-Type missing", 1);
	setenv("CONTENT_TYPE", content_type.c_str(), 1);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	
	int fd_body_tmp = getRequest().getFdBody();
	if (fd_body_tmp >= 0) {
		dup2(fd_body_tmp, STDIN_FILENO);
		close(fd_body_tmp);
	}
}