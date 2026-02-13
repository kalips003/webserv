#ifndef HTTPR_HPP
#define HTTPR_HPP

#include "HttpObj.hpp"

#define MAX_HEXA_LINE_LENGTH 32

///////////////////////////////////////////////////////////////////////////////]
class HttpRequest : public HttpObj {

private:
///////////////////////////////////////////////////////////////////////////////]
	std::string		_method; // GET
	std::string		_path; // /index.html
	std::string		_version; // HTTP/1.1
///////////////////////////////////////////////////////////////////////////////]

public:
	HttpRequest(const Settings::server_setting* settings) : HttpObj(settings) { _status = READING_FIRST; }


//-----------------------------------------------------------------------------]
	/***  VIRTUALS  ***/
public:
	virtual int		isFirstLineValid(int fd);
	virtual int		parseHeadersForValidity();

///////////////////////////////////////////////////////////////////////////////]
private:
	int			validateLocationBlock(ssize_t body_size);
	int			readBodyChunk(char *buff, size_t sizeofbuff, int fd, ReadFunc reader);
	int			streamingBodyWrapper(char *buff, size_t sizeofbuff, int fd, ReadFunc reader);
	int			detectChunkedEncoding();
	int			readBodyChunk_delimHelper(char *buff, size_t sizeofbuff, int fd, ReadFunc reader);

///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
public:
	const std::string&	getMethod() const { return _method; }
	const std::string&	getPath() const { return _path; }
	const std::string&	getVersion() const { return _version; }
	temp_file&			getTempFile() { return _tmp_file; }
//-----------------------------------------------------------------------------]
/***  SETTERS  ***/
public:
///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	friend std::ostream& operator<<(std::ostream& os, const HttpRequest& r);
};

std::ostream& operator<<(std::ostream& os, const HttpRequest& r);

#endif

/*****			HTTP REQUEST HEADERS
1️⃣ HTTP Request Headers
Accept:                 MIME types the client can handle (text/html, application/json)
Accept-Charset:         Character sets the client can handle (utf-8, iso-8859-1)
Accept-Encoding:        Compression formats the client accepts (gzip, deflate, br)
Accept-Language:        Preferred languages (en-US, fr)
Authorization:          Credentials for HTTP authentication (Basic, Bearer tokens)
Cache-Control:          Client caching rules (no-cache, max-age=0)
Connection:             keep-alive or close
Content-Length:         Length of the request body in bytes
Content-Type:           MIME type of the request body (application/json, multipart/form-data)
Cookie:                 Cookies sent to the server
Date:                   Date and time of the request
Expect:                 Indicates expectations, e.g., 100-continue
From:                   Email of the user making the request (rare)
Host:                   Mandatory in HTTP/1.1, server domain
If-Match:               Conditional request based on ETag
If-Modified-Since:      Only send if resource changed since date
If-None-Match:          Only send if ETag does not match
If-Range:               Used for partial content requests
If-Unmodified-Since:    Only send if resource not modified since date
Max-Forwards:           Limit for proxies and TRACE requests
Origin:                 Origin of cross-site request (CORS)
Pragma:                 Legacy caching instructions (no-cache)
Proxy-Authorization:    Credentials for proxy authentication
Range:                  Request part of a resource (bytes=0-499)
Referer:                URL of the page making the request
TE:                     Transfer encodings the client accepts
Upgrade:                Request protocol upgrade (websocket)
User-Agent:             Software making the request (browser, curl)
Via:                    Proxies the request went through
Warning:                General warnings
*/
