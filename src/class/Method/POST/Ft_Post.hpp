#ifndef FT_POST_HPP
#define FT_POST_HPP

#include "Method.hpp"

#include <fstream>
///////////////////////////////////////////////////////////////////////////////]
//* GET *     | Request a resource  | No body (technically allowed but ignored)|
///////////////////////////////////////////////////////////////////////////////]

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

// GET /index.html HTTP/1.1
// Host: example.com
// User-Agent: curl/8.0
// Accept: text/html
// Accept-Encoding: gzip, deflate
// Connection: keep-alive
class   Ft_Post : public Method {

private:
	temp_file	treated_file;

public:
	Ft_Post(const t_connec_data& data) : Method(data) {}

// parent virtual funcitons:
	void	printHello();
	int		exec_cgi();
	int		howToHandleFileNotExist(const std::string& ressource, int rtrn_open);
	int		handleFileExist(std::string& ressource);
	int		handleDir(std::string& ressource);
	void	prepareChild(const std::string& ressource, const std::string& query);
	int		appendFile(const std::string& path);

	void setlocationBlock(block* b) { _location_block = b; }
private:
	int	treatMultipart();

};



#endif