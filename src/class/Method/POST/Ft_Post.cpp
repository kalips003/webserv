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
// int		Ft_Post::treatMultipart(std::string& ressource) {

// // IS Content-Type = multipart/form-data?
// // 		if yes, extract boundary 
// 	const std::string* c_type = _request.find_in_headers("Content-Type");
// 	if (!c_type)
// 		return ; // some http error content type missing

// // Content-Type: multipart/form-data; boundary=abc; charset=UTF-8
// // Content-Type: multipart/form-data; boundary=----geckoformboundaryeb47a963ac0d54a6202a6abec127252
// 	size_t pos = (*c_type).find("multipart/form-data");
// 	if (pos == std::string::npos)
// 		return ; // not multipart
	
// 	std::string delim;
// 	std::vector<std::string> v = splitOnDelimitor(*c_type, ";");
// 	const std::string b("boundary=");
// 	for (std::vector<std::string>::const_iterator it = v.begin(); it != v.end(); ++it) {

// 		size_t pos;
// 		if ((pos = (*it).find(b)) != std::string::npos) {
// 			delim = "--" + trim_any((*it).substr(pos + b.size()), " \"\'");
// 			break;
// 		}
// 	}
// 	if (delim.empty())
// 		return ; // error






// 	_request.getFile().resetFileFd();
// 	_request.getFile().updateStat();
// 	HttpMultipart onePartMan(delim, "");
// 	onePartMan.setBytesTotal(_request.getFile()._info.st_size);

// 	int fd = _request.getFile()._fd;
// 	HttpObj::HttpBodyStatus status;
// 	while ((status = onePartMan.getStatus()) != HttpObj::DOING) {

// 		int rtrn = onePartMan.parse_multifile(_data._buffer, _data._sizeofbuff, fd);

// 		if (status == HttpObj::CLOSED) // eof found
// 			return 400;
// 		if (status >= 100)
// 			return status;
		
// 	}

// /*
// 	--BOUNDARY\r\n
// 	<part headers>\r\n
// 	\r\n
// 	<part body>
// 	\r\n
// 	--BOUNDARY--
//  */
// 	std::string& leftover = onePartMan.getLeftovers();
// 	// check _first == delim
// 	if (onePartMan.getFirst() != delim)
// 		return 400;
// 	// next 2 char == "--"
// 	if (leftover.size() < delim.size() + 2)
// 		return 400;
// 	if (leftover[delim.size()] != '-' && leftover[delim.size() + 1] != '-')
// 		return 400;
// 	// check headers for filename?
// 	if (!onePartMan.getFilename().empty()) {
// 		size_t pos = ressource.find_last_of("/");

// 		// /root_path/web_cat/donations/image.ico
// 		std::string file_to_upload = ressource.substr(0, pos + 1) + onePartMan.getFilename();
		

// 		int rtrn = Method::handleFile();
// 		// rename(onePartMan.getFilename().empty());
// 	} else {
// 		// ignored metadata
// 	}
// 	// content type?




// // /web_cat/donations/file2
// // > /root_path//web_cat/donations/file2
// // > /root_path//web_cat/donations/icon052.ico


// ////////
// 	std::vector<HttpMultipart> body_parts;
// 	body_parts.push_back(HttpMultipart(delim, ""));
// 	body_parts.back().getFile().updateStat();
// 	body_parts.back().setBytesTotal(_request.getFile()._info.st_size);

// 	bool finished = false;
// 	while (!finished) {
// 		int rtrn = body_parts.back().parse_multifile(_data._buffer, _data._sizeofbuff, _request.getFile()._fd);
// 		if (rtrn >= 100)
// 			return rtrn;
// 		if (rtrn == HttpObj::CLOSED) // EOF found before delim
// 			return 400;
// 		if (rtrn ==  HttpObj::DOING) { // this instance found (--BOUNDARY) _buffer the last read, _leftovers the rest

// 			std::string& leftover = body_parts.back().getLeftovers();
// 			if (leftover.size() >= boundary.size() + 4) {
// 				if (leftover[boundary.size() + 3] == '\r' && leftover[boundary.size() + 3] == '\n')
// 					; // write _buffer into file = file is finished
// 					; // remove \r\n from _leftover
// 					; // push_back new Multipart(body_parts.back())
// 				else if (leftover[boundary.size() + 3] == '-' && leftover[boundary.size() + 3] == '-')
// 					if (stuff after, is it malformed?)
// 					; // write _buffer into file = file is finished
// 					; // finished
			
			
// 			} else {
// 				read(_request.getFile()._fd, _data._buffer, 2);
			
// 			}
		



// 		}
// 	// }

// 	// after reading headers and such
// 	// obj.setBytesWritten(0);
// 	// obj.getFile().createTempFile(&g_settings.getTempRoot());
// 	return 0;
// }

// int helper()