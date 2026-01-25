#include "Ft_Post.hpp"

#include "Tools1.cpp"
#include "Tools2.cpp"
#include "HttpAnswer.cpp"

///////////////////////////////////////////////////////////////////////////////]
void Ft_Post::printHello() {
	printLog(DEBUG, "Post method called", 1);
}

/**	Funciton called on second loop, once _cgi_status == CGI_DOING, 
* child is set-up and so on 
* @return -1 if cgi still going 
* @return 0 if cgi finished (and handled)
* @return ErrCode in the case of any error
*/
int		Ft_Post::exec_cgi() {
	printLog(ERROR, "--> you have to do this part (execcgi)", 1);
	return 0;
}

/**	rtrn_open == 404 / 403
* In Post, reject file existance?  */
int		Ft_Post::howToHandleFileNotExist(const std::string& ressource, int rtrn_open) {

	if (rtrn_open == 404) {

		const std::string tmp_path = getRequest().getBodyPath();
		if (tmp_path.empty())
			; // the request didnt include a body
		if (rename(tmp_path.c_str(), ressource.c_str()) < 0) {
			printErr("rename()");
			return 500;
		}
		getRequest().set

		return 201;
	}
	else {// if (rtrn_open == 403)
		printLog(WARNING, "Post request File already exist: permission error", 1);
		return 403;
	}
}

/** */
int		Ft_Post::handleFile(std::string& path, struct stat ressource_info) {

	if (access(path.c_str(), W_OK) != 0) // even if file exist, might not be readable by server
		return 403;

	int flags_open = O_WRONLY;
	const std::string* post_policy = g_settings.find_setting_in_blocks("location", "/upload", "post_policy");
	if (!post_policy || *post_policy == "reject") {
		printLog(WARNING, "Post request File already exist: post_policy = reject", 1);
		return 409;
	}
	else if (*post_policy == "replace") // open(O_WRONLY | O_CREAT | O_TRUNC)
		flags_open |= O_TRUNC | O_CREAT;
	else if (*post_policy == "append") // open(O_WRONLY | O_CREAT | O_APPEND)
		flags_open |= O_APPEND | O_CREAT;
	else {
		printLog(WARNING, "Post request File already exist: post_policy = unknown", 1);
		return 500;
	}


	
	return 0;
}

/** */
int		Ft_Post::handleDir(std::string& ressource) {

	return 403; // or 405 with header {Allow: GET, HEAD}
}

#include "HttpRequest.hpp"
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