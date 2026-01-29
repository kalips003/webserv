#ifndef TASK_HPP
#define TASK_HPP

#include "Log.hpp"

class Connection;
class Server;
class HttpRequest;
class HttpAnswer;
class SettingsServer;

#include <string>
#include <sys/stat.h>

#include "SettingsServer.hpp"
#include "Connection.hpp"
#include "TempFile.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////]
/*   Method   | Purpose / Client asks                         | Body allowed?                                |
| ----------- | --------------------------------------------- | -------------------------------------------- |
| **GET**     | Request a resource                            | No body (technically allowed but ignored)    |
| **HEAD**    | Request headers of resource                   | No body                                      |
| **POST**    | Submit data to server (form, JSON, file)      | Usually yes, body contains data              |
| **PUT**     | Replace resource at URL                       | Usually yes, body contains full new resource |
| **PATCH**   | Partial update of resource                    | Usually yes, body contains update            |
| **DELETE**  | Remove resource                               | Rarely has body; most servers ignore it      |
| **OPTIONS** | Ask server for allowed cmethods / capabilities | Can have body, but uncommon                 |
| **CONNECT** | Ask server to open a tunnel (HTTPS proxy)     | No body; used to establish tunnel            |
| **TRACE**   | Echo back the request                         | No body                                      |
////////////////////////////////////////////////////////////////////////////////////////////////////////////]*/

typedef Connection::transfer_data	t_connec_data;
// static_assert(sizeof(t_connec_dataa) != sizeof(int), "t_connec_data is int");

///////////////////////////////////////////////////////////////////////////////]
// 							VIRTUAL CLASS: TASK
///////////////////////////////////////////////////////////////////////////////]
class Method {

public:
	struct cgi_data {
		temp_file		_tmp_file;
		int				_child_pipe_fd;
		pid_t			_child_pid;

		cgi_data() : _tmp_file(), _child_pipe_fd(-1), _child_pid(-1) {}
	};

	enum Ft_Type {
		GET,
		POST,
		DELETE,
		PUT,
		UNKNOWN		
	};

///////////////////////////////////////////////////////////////////////////////]
private:
	HttpRequest&			_request;
	HttpAnswer&				_answer;

	const t_connec_data&	_data;
	cgi_data				_cgi_data;
	const block*			_location_block;
///////////////////////////////////////////////////////////////////////////////]

public:
			Method(const t_connec_data& data) : 
				_request(data._this_ptr->getRequest()), 
				_answer(data._this_ptr->getAnswer()), 
				_data(data),
				_cgi_data(), 
				_location_block(NULL) {}

	virtual	~Method();


//-----------------------------------------------------------------------------]
public:
	int 		normal_doing();
	int			iniCGI(const std::string& ressource, const std::string& query, const std::string* CGI_interpreter_path);


//-----------------------------------------------------------------------------]
	/***  STATICS  ***/
public:
	static Ft_Type parseMethod(const std::string& method);
	static Method* createTask(const std::string& method, const t_connec_data& data);

//-----------------------------------------------------------------------------]
	/***  TOOLS  ***/
public:
	static int			isFileNOK(std::string path, struct stat& ressource_info);
	const std::string*	isCGI(const std::string& path) const;
	int 				getFullPath(std::string& path_to_fill, const std::string& given_path) const;
	int 				sanitizePath(std::string& path_to_fill, const std::string& given_path) const;
	const block*		isLocationKnown(const std::string& given_path) const;

///////////////////////////////////////////////////////////////////////////////]
/***							VIRTUAL FUNCTIONS							***/
///////////////////////////////////////////////////////////////////////////////]
/**	Print Debug the name of the Derived method class */
	virtual void	printHello() = 0;
//-----------------------------------------------------------------------------]
/**	Function called on second loop of DOING, once _cgi_status == CGI_DOING, 
* child is already set-up by first loop 
* @return -1 if cgi still going 
* @return 0 if cgi finished (and handled)
* @return ErrCode in the case of any error		---*/
	virtual int		exec_cgi() = 0;
//-----------------------------------------------------------------------------]
/** If the requested path doesnt exist, how should the Method handle it?
* @param ressource: the cleaned absolute path of the request
* @param rtrn_open: the errCode to handle (404 / 403)
* @return	Must return directly the ft_do return 
*	(0 on success, or errCode)	---*/
	virtual int		howToHandleFileNotExist(const std::string& ressource, int rtrn_open) = 0;
//-----------------------------------------------------------------------------]
/** If the file exist and inst a cgi script, how should the Method handle it?
* @param ressource: the cleaned absolute path of the request
* @param ressource_info: initialized stat struct of ressource
* @return	Must return directly the correct ft_do return 
*	(0 on success, or errCode)	---*/
	virtual int		handleFile(std::string& ressource) = 0;
//-----------------------------------------------------------------------------]
/** If the ressource exist and is a Directory, how should the Method handle it?
* @param ressource: the cleaned absolute path of the request
* @return	Must return directly the correct ft_do return 
*	(0 on success, or errCode)	---*/
	virtual int		handleDir(std::string& ressource) = 0;
//-----------------------------------------------------------------------------]
/** Method specific preparation of the CGI_Child before execve()
* @param ressource: the cleaned absolute path of the requested script to run
* @param query: the query part, extracted from request._path (= ?x=abc&y=42)
* @return	Must return directly the correct ft_do return 
*	(0 on success, or errCode)	---*/
	virtual void	prepareChild(const std::string& ressource, const std::string& query) = 0;
///////////////////////////////////////////////////////////////////////////////]


///////////////////////////////////////////////////////////////////////////////]
	/***  GETTERS  ***/
public:
	HttpRequest&			getRequest() const { return _request; }
	HttpAnswer&				getAnswer() { return _answer; }
	const t_connec_data&	getData() { return _data; }
	cgi_data&				getCGIData() { return _cgi_data; }
	const block*			getLocationBlock() const { return _location_block; }

///////////////////////////////////////////////////////////////////////////////]
	/***  SETTERS  ***/
public:
///////////////////////////////////////////////////////////////////////////////]

};

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