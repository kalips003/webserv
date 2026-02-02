#include "Ft_Post.hpp"

#include "Log.hpp"
#include "Tools1.hpp"
#include "Tools2.hpp"
#include "HttpAnswer.hpp"
#include "HttpRequest.hpp"

///////////////////////////////////////////////////////////////////////////////]
void Ft_Post::printHello() {
	LOG_DEBUG("POST method called");
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

	if (access(folder_path.c_str(), W_OK | X_OK) != 0) {// need write / exec permissions
		LOG_DEBUG("Ft_Post::howToHandleFileNotExist() cant access: " << folder_path);
		return 403;
	}
//	
	if (rtrn_open == 404) {

		const std::string& tmp_path = _request.getFile()._path;
		if (tmp_path.empty())
			return 400; // the request didnt include a body

		if (rename(tmp_path.c_str(), ressource.c_str()) < 0) {
			LOG_ERROR("rename()");
			return 500;
		}
		// HttpRequest still own the fd and /path of the temp, but cleared in destructor
		_answer.getFile().closeTemp(false); // can also remove it manually now

		_answer.setFirstLine(201);
		return Connection::SENDING;
	}
	else {// if (rtrn_open == 403)
		LOG_WARNING("Post request File already exist (" << ressource << "): permission error");
		return 403;
	}
}

///////////////////////////////////////////////////////////////////////////////]
int		Ft_Post::handleFileExist(std::string& path) {

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
		LOG_WARNING("Attempt to over-write an existing file (POST; post_policy = reject): " C_410 << path << RESET);
		return 409;
	}
	else if (post_policy == "replace") { // open(O_WRONLY | O_CREAT | O_TRUNC)
		const std::string& tmp_path = _request.getFile()._path;
		if (rename(tmp_path.c_str(), path.c_str()) < 0) { // if file already exist, replace it silently
			LOG_ERROR("rename()");
			return 500;
		}
		_request.getTempFile().closeTemp(false);
	}
	else if (post_policy == "append")
		return appendFile(path);
	else {
		LOG_WARNING("Post request File already exist: post_policy = unknown");
		return 500;
	}
	return Connection::SENDING;
}

#include <fcntl.h>
///////////////////////////////////////////////////////////////////////////////]
/**	reset _request.fd to pos 0, create a temp fd for dest in append mode, loop on copy.
// @note after the copy, the _request.temp stays untouched  */
int		Ft_Post::appendFile(const std::string& path) {// open(O_WRONLY | O_CREAT | O_APPEND)


	int src_fd = _request.getFile()._fd;
	if (src_fd < 0)
	{
		LOG_ERROR("appendFile(): You shouldnt see this");
		return 500;
	}

// reset fd to the start
	lseek(src_fd, 0, SEEK_SET);

	int dest_fd = open(path.c_str(), O_WRONLY | O_APPEND);
	if (dest_fd < 0) return 500;

	char buf[4096];
	ssize_t n;
	while ((n = read(src_fd, buf, sizeof(buf))) > 0) {
		if (write(dest_fd, buf, n) != n) {
			LOG_ERROR("Append from: " << _request.getFile()._path << " to: " << path << " failed\n");
			close(dest_fd);
			return 500; // partial write happened
		}
	}
	if (n < 0) {
		LOG_ERROR("Append from: " << _request.getFile()._path << " to: " << path << " failed\n");
		close(dest_fd);
		return 500;
	}

	close(dest_fd);
	return Connection::SENDING;
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

	std::string content_length = itostr(_request.getFile().getBodySize());
	setenv("CONTENT_LENGTH", content_length.c_str(), 1);
	const std::string* content_type = _request.find_in_headers("content-type");
	if (!content_type)
		LOG_WARNING("CGI POST, Content-Type missing");
	setenv("CONTENT_TYPE", (*content_type).c_str(), 1);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	
	int fd_body_tmp = _request.getFile()._fd;
	if (fd_body_tmp >= 0) {
		lseek(fd_body_tmp, 0, SEEK_SET);
		dup2(fd_body_tmp, STDIN_FILENO);
		close(fd_body_tmp);
	}
}