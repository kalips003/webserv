#ifndef TASK_HPP
#define TASK_HPP

class Connection;
class Server;
class HttpRequest;
class httpAnswer;
class SettingsServer;

#include <string>
#include <sys/stat.h>

#include "SettingsServer.hpp"
///////////////////////////////////////////////////////////////////////////////]
enum CgiStatus {
	CGI_NONE = 0,
	CGI_DOING,
};

struct transfer_data {
    int         _client_fd; // fd associated with this client connection
	int			_epoll_fd;
	Connection*	_this_ptr; // ptr to this client connection

	transfer_data() : _client_fd(-1), _epoll_fd(-1), _this_ptr(NULL) {}
};

struct cgi_data {
	std::string		_tmp_file_name;
	int				_tmp_file_fd;
	int				_child_pipe_fd;
	pid_t			_child_pid;

	cgi_data() : _tmp_file_fd(-1), _child_pipe_fd(-1), _child_pid(-1) {}
};


///////////////////////////////////////////////////////////////////////////////]
class Task {

private:
	std::string				_buffer; 
	HttpRequest&			_request;
	httpAnswer&				_answer;

	// int						_status; // 404
	CgiStatus				_cgi_status;
	
	transfer_data			_data;
	cgi_data				_cgi_data;
	const block*			_location_block;

public:
	static Task* createTask(const std::string& method, Connection& connec, int epoll_fd);
	virtual ~Task();

public:
	Task(Connection& connec, int epoll);

	int			ft_do();
	virtual void	printHello() = 0;
	virtual int		exec_cgi() = 0;
	virtual int		howToHandleFileNotExist(const std::string& ressource, int rtrn_open) = 0;
	virtual int		handleFile(std::string& ressource, struct stat ressource_info) = 0;
	virtual int		handleDir(std::string& ressource) = 0;
	virtual void	prepareChild(const std::string& ressource, const std::string& query) = 0;
	int 		normal_doing();
	int			iniCGI(const std::string& ressource, const std::string& query, const std::string* CGI_interpreter_path);

//-----------------------------------------------------------------------------]
	static int	isFileNOK(std::string path, struct stat& ressource_info);

	const std::string* isCGI(const std::string& path) const;
	int getFullPath(std::string& path_to_fill, const std::string& given_path) const;
	int sanitizePath(std::string& path_to_fill, const std::string& given_path) const;
	const block* isLocationKnown(const std::string& given_path) const;


///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
    const std::string& getBuffer() const { return _buffer; }
    const HttpRequest& getRequest() const { return _request; }
    httpAnswer& getAnswer() { return _answer; }
    transfer_data& getData() { return _data; }
// 
    // int 	getStatus() const { return _status; }
    cgi_data&	getCGIData() { return _cgi_data; }
    CgiStatus 	getCGIStatus() const { return _cgi_status; }
	const block* getLocationBlock() const { return _location_block; }

/***  SETTERS  ***/
    // void 	setStatus(int status)  { _status = status; }
    void  	addBuffer(std::string& s) { _buffer += s; };
    void 	setCGIStatus(CgiStatus status)  { _cgi_status = status; }
///////////////////////////////////////////////////////////////////////////////]
};

///////////////////////////////////////////////////////////////////////////////]
/*
| Method      | Purpose / Client asks                         | Body allowed?                                |
| ----------- | --------------------------------------------- | -------------------------------------------- |
| **GET**     | Request a resource                            | No body (technically allowed but ignored)    |
| **HEAD**    | Request headers of resource                   | No body                                      |
| **POST**    | Submit data to server (form, JSON, file)      | Usually yes, body contains data              |
| **PUT**     | Replace resource at URL                       | Usually yes, body contains full new resource |
| **PATCH**   | Partial update of resource                    | Usually yes, body contains update            |
| **DELETE**  | Remove resource                               | Rarely has body; most servers ignore it      |
| **OPTIONS** | Ask server for allowed cmethods / capabilities | Can have body, but uncommon                  |
| **CONNECT** | Ask server to open a tunnel (HTTPS proxy)     | No body; used to establish tunnel            |
| **TRACE**   | Echo back the request                         | No body                                      |
*/

///////////////////////////////////////////////////////////////////////////////]
/*
| Header              | Purpose                                                                   |
| ------------------- | ------------------------------------------------------------------------- |
| `Host`              | mandatory in HTTP/1.1, specifies which host the request targets           |
| `User-Agent`        | client info (browser, curl, etc.)                                         |
| `Accept`            | media types the client can handle (`text/html`, `application/json`, etc.) |
| `Accept-Encoding`   | compression formats client supports (`gzip`, `deflate`, `br`)             |
| `Accept-Language`   | preferred languages                                                       |
| `Connection`        | e.g., `keep-alive`, `close`                                               |
| `Cache-Control`     | caching directives, e.g., `no-cache`, `max-age=0`                         |
| `If-Modified-Since` | conditional GET: only send if resource modified since given date          |
| `If-None-Match`     | conditional GET using ETag                                                |
| `Referer`           | URL of the page linking to this resource                                  |
| `Cookie`            | session cookies, auth tokens                                              |
*/



#endif