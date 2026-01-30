#include "Ft_Post.hpp"

#include "Tools1.hpp"

#include "Multipart.hpp"
///////////////////////////////////////////////////////////////////////////////]
/*
	--BOUNDARY\r\n
	<part headers>\r\n
	\r\n
	<part body>
	\r\n
	--BOUNDARY\r\n
	<part headers>\r\n
	\r\n
	<part body>
	\r\n
	--BOUNDARY--\r\n
 */
 /* 		filename present → file | no filename → regular form field
	Content-Disposition: form-data; name="file"; filename="a.txt"\r\n // mandatory
	Content-Type: text/plain\r\n // Content-Type is optional (default: text/plain)
	\r\n
	hello world\n
	\r\n
*/
void	Ft_Post::treatMultipart() {

	const std::string* c_type = _request.find_in_headers("Content-Type");
	if (!c_type)
		return ; // some http error content type missing

// Content-Type: multipart/form-data; boundary=abc; charset=UTF-8
// Content-Type: multipart/form-data; boundary=----geckoformboundaryeb47a963ac0d54a6202a6abec127252
	size_t pos = (*c_type).find("multipart/form-data");
	if (pos == std::string::npos)
		return ; // not multipart
	
	std::string boundary;
	std::vector<std::string> v = splitOnDelimitor(*c_type, ";");
	const std::string b("boundary=");
	for (std::vector<std::string>::const_iterator it = v.begin(); it != v.end(); ++it) {

		size_t pos;
		if ((pos = (*it).find(b)) != std::string::npos) {
			boundary = "--" + trim_any((*it).substr(pos + b.size()), " \"\'");
			break;
		}
	}
	if (boundary.empty())
		return ; // error

	HttpMultipart obj(boundary);

}
