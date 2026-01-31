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
int		Ft_Post::treatMultipart() {

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

	_request.getFile().resetFileFd();
	std::vector<HttpMultipart> body_parts;
	body_parts.push_back(HttpMultipart(boundary, ""));
	body_parts.back().getFile().updateStat();
	body_parts.back().setBytesTotal(_request.getFile()._info.st_size);

	bool finished = false;
	while (!finished) {
		int rtrn = body_parts.back().parse_multifile(_data._buffer, _data._sizeofbuff, _request.getFile()._fd);
		if (rtrn >= 100)
			return rtrn;
		if (rtrn == HttpObj::CLOSED) // EOF found before delim
			return 400;
		if (rtrn ==  HttpObj::DOING) { // this instance found (--BOUNDARY) _buffer the last read, _leftovers the rest

			std::string& leftover = body_parts.back().getLeftovers();
			if (leftover.size() >= boundary.size() + 4) {
				if (leftover[boundary.size() + 3] == '\r' && leftover[boundary.size() + 3] == '\n')
					; // write _buffer into file = file is finished
					; // remove \r\n from _leftover
					; // push_back new Multipart(body_parts.back())
				else if (leftover[boundary.size() + 3] == '-' && leftover[boundary.size() + 3] == '-')
					if (stuff after, is it malformed?)
					; // write _buffer into file = file is finished
					; // finished
			
			
			} else {
				read(_request.getFile()._fd, _data._buffer, 2);
			
			}
		



		}
	
	

	
	}

	// after reading headers and such
	obj.setBytesWritten(0);
	obj.getFile().createTempFile(&g_settings.getTempRoot());

}

int helper()