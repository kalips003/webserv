#include "Ft_Post.hpp"
#include "_colors.h"

#include "Tools1.hpp"
#include "HttpRequest.hpp"
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

// 	const std::string* c_type = _request.find_setting("content-type");
// 	if (!c_type)
// 		; // some http error content type missing

// // Content-Type: multipart/form-data; boundary=abc; charset=UTF-8
// // Content-Type: multipart/form-data; boundary=----geckoformboundaryeb47a963ac0d54a6202a6abec127252
// 	size_t pos = (*c_type).find("multipart/form-data;");
// 	if (pos == std::string::npos)
// 		; // not multipart
	
// 	std::string boundary;
// 	std::vector<std::string> v = splitOnWhite(*c_type);
// 	for (std::vector<std::string>::const_iterator it = v.begin(); it != v.end(); ++it) {

// 		const std::string b("boundary=");
// 		size_t pos;
// 		if ((pos = (*it).find("boundary=")) != std::string::npos) {
// 			boundary = "--" + (*it).substr(pos + b.size());
// 			break;
// 		}
// 	}
// 	if (boundary.empty())
// 		; // error

	
// 	HttpObj obj;
// 	obj.readMultipart(_data._buffer, _data._sizeofbuff, _request.getFile()._fd, boundary);



// 	std::string end_boundary = boundary + "--";
// 	char	buffer[4096];
// 	temp_file&	tmp = getRequest().getFile();
// 	tmp.resetFileFd();
// 	tmp.updateStat();
// 	size_t tmp_size = tmp._info.st_size;

// 	bool isfound;
// 	ssize_t bytes_read_total = 0;
// 	ssize_t bytes_read;
	
// 	while (bytes_read_total < tmp_size) {
// 		bytes_read = obj.read_until_delim_is_found(buffer, sizeof(buffer), tmp._fd, boundary, isfound);
// 		if (isfound || !bytes_read)
// 			break; // eof
// 		if (bytes_read > 0)
// 			bytes_read_total += bytes_read;
// 	}
// // now anything before boundary is in _buffer
// // everything else (including boundary) is in _leftover
// 	obj.getBuffer().clear();
// 	obj.getLeftovers().erase(boundary.size()); // now only everything after boundary is in _leftover

// 	obj.readingHeaders();
// 	while (bytes_read_total < tmp_size) {

// 	}

}
