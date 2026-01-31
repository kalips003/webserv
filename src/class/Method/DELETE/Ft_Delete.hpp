#ifndef FT_DELETE_HPP
#define FT_DELETE_HPP

#include "Method.hpp"

#include <fstream>
///////////////////////////////////////////////////////////////////////////////]
//* DELETE *  | Remove a resource   | Body is usually ignored, but some servers accept metadata in body |
///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
/*
| Header              | Purpose                                                                    |
| ------------------- | -------------------------------------------------------------------------- |
| `Host`              | mandatory in HTTP/1.1, specifies which host the request targets            |
| `User-Agent`        | client info (browser, curl, etc.)                                          |
| `Accept`            | media types the client can handle (`text/html`, `application/json`, etc.)  |
| `Accept-Encoding`   | compression formats client supports (`gzip`, `deflate`, `br`)              |
| `Accept-Language`   | preferred languages                                                        |
| `Connection`        | e.g., `keep-alive`, `close`                                                |
| `Cache-Control`     | caching directives, e.g., `no-cache`, `max-age=0`                          |
| `If-Match`          | only delete if resource’s ETag matches this value (prevents accidental delete) |
| `If-Unmodified-Since` | only delete if resource has not been modified since given date          |
| `Authorization`     | credentials if deletion requires authentication                             |
| `Cookie`            | session cookies, auth tokens                                               |
*/

// GET /index.html HTTP/1.1
// Host: example.com
// User-Agent: curl/8.0
// Accept: text/html
// Accept-Encoding: gzip, deflate
// Connection: keep-alive
class   Ft_Delete : public Method {

private:


public:
	Ft_Delete(const t_connec_data& data) : Method(data) {}

// parent virtual funcitons:
	void	printHello();
	int		exec_cgi();
	int		howToHandleFileNotExist(const std::string& ressource, int rtrn_open);
	int		handleFileExist(std::string& ressource);
	int		handleDir(std::string& ressource);
	void	prepareChild(const std::string& ressource, const std::string& query);

private:
	
};



#endif