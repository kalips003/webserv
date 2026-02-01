#ifndef HTTPA_HPP
#define HTTPA_HPP

#include "HttpObj.hpp"

///////////////////////////////////////////////////////////////////////////////]
// "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n<body>"
///////////////////////////////////////////////////////////////////////////////]
class HttpAnswer : public HttpObj {

private:
///////////////////////////////////////////////////////////////////////////////]
	std::string		_version; // HTTP/1.1
	int				_status_num; // 200
	std::string		_status_msg; // OK
///////////////////////////////////////////////////////////////////////////////]

public:
	HttpAnswer() : HttpObj(), _version("HTTP/1.1"), _status_num(200), _status_msg("OK") { _status = SENDING_HEAD; }

//-----------------------------------------------------------------------------]
	/***  VIRTUALS  ***/
public:
	int		isFirstLineValid(int fd);

///////////////////////////////////////////////////////////////////////////////]
public:
	void	createError(int errCode);
	void	setFirstLine(int errCode);
	void	initializationBeforeSend();


///////////////////////////////////////////////////////////////////////////////]
	/***  GETTERS  ***/
public:
	const std::string&	getVersion() { return _version; }
	int					getStatusNum() { return _status_num; }
	const std::string&	getStatusMsg() { return _status_msg; }
	temp_file&			getTempFile() { return _tmp_file; }
//-----------------------------------------------------------------------------]
private:

///////////////////////////////////////////////////////////////////////////////]
	/***  SETTERS  ***/
public:
//-----------------------------------------------------------------------------]
private:
///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	friend std::ostream& operator<<(std::ostream& os, const HttpAnswer& r);
};

std::ostream& operator<<(std::ostream& os, const HttpAnswer& r);

#endif

/*****			HTTP ANSWER HEADERS
1️⃣ HTTP Response Headers
Accept-Ranges:                 Indicates if server supports range requests (bytes)
Age:                           Age of the cached response in seconds
Allow:                         Methods allowed for the resource (GET, POST, OPTIONS)
Cache-Control:                 Caching rules (no-store, private, max-age)
Connection:                     keep-alive or close
Content-Encoding:               Compression format applied (gzip, br)
Content-Language:               Language of the content (en-US)
Content-Length:                 Length of the response body in bytes
Content-Location:               Alternate location for the returned content
Content-Disposition:            How content should be handled (inline, attachment; filename="file.txt")
Content-Type:                   MIME type of response (text/html, application/json)
Date:                           Date/time of response
ETag:                           Unique identifier for version of resource
Expires:                        Expiration date for caching
Last-Modified:                  Last modification date of resource
Location:                       Redirect target URL (3xx responses)
Pragma:                         Legacy caching instructions (no-cache)
Retry-After:                    When to retry (503 maintenance)
Server:                         Server software/version (nginx/1.25)
Set-Cookie:                     Cookie sent to client
Strict-Transport-Security:      HSTS rules (HTTPS only)
Trailer:                        Headers sent after chunked body
Transfer-Encoding:              Encoding applied (chunked)
Upgrade:                        Protocol upgrades (websocket)
Vary:                           Specifies headers that affect caching (Accept-Encoding)
Via:                            Proxy chain
Warning:                        General warnings
WWW-Authenticate:               Challenge for authentication (Basic, Digest)

 */