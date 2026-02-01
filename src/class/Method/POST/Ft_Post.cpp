#include "Ft_Post.hpp"

#include "Tools1.hpp"

#include "Multipart.hpp"
///////////////////////////////////////////////////////////////////////////////]
/*
	Content-Disposition: form-data; name="file"; filename="a.txt"\r\n // mandatory
	filename present → file | no filename → regular form field

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
///////////////////////////////////////////////////////////////////////////////]
/** Implementation of Method::treatContentType (that return -1)
* here to treat "Content-Type" (implemented, only multipart/form-data) 
* this function find content type == multipart/form-data and recover the boundary
* the next one does the splitting and treating of _request._tmp_file
* @return -1 if no multipart, errCode on any error, Connection::SENDING if all goes well */
int		Ft_Post::treatContentType(std::string& ressource, std::string& query) {

// IS Content-Type = multipart/form-data?
// 		if yes, extract boundary 
	const std::string* c_type = _request.find_in_headers("Content-Type");
	if (!c_type)
		return -1; // treat as raw bytes

// Content-Type: multipart/form-data; boundary=abc; charset=UTF-8
	size_t pos = (*c_type).find("multipart/form-data");
	if (pos == std::string::npos)
		return -1; // not multipart / others not handled
	
	std::string delim;
	std::vector<std::string> v = splitOnDelimitor(*c_type, ";");
	const std::string b("boundary=");
	for (std::vector<std::string>::const_iterator it = v.begin(); it != v.end(); ++it) {

		size_t pos;
		if ((pos = (*it).find(b)) != std::string::npos) {
			delim = "--" + trim_any((*it).substr(pos + b.size()), " \"\'");
			break;
		}
	}
	if (delim.empty())
		return 400; // error

	return treatMultipart(delim, ressource, query);
}

///////////////////////////////////////////////////////////////////////////////]
/** Parses a multipart/form-data request body, splits it into individual parts,
 * and processes each part (file or metadata) against the target resource.
 *
 * Returns 0 on success, or an HTTP error code on malformed input or failure
 * while receiving or handling multipart sections. 	---*/
int		Ft_Post::treatMultipart(std::string& delim, std::string& ressource, std::string& query) {

// Setup Vector
	_request.getFile().resetFileFdBegining();
	int fd_original = _request.getFile()._fd;

	std::vector<HttpMultipart> body_parts;
	body_parts.reserve(10);
	body_parts.push_back(HttpMultipart(delim));

	HttpMultipart* current = &body_parts.back();
	current->setBytesTotal(_request.getFile().getBodySize());

// loop fills vector
	HttpObj::HttpBodyStatus status = body_parts.back().getStatus();
	while (status != HttpObj::DOING) {
		int rtrn = current->receive(_data._buffer, _data._sizeofbuff, fd_original, ::read);
		if (rtrn >= 100)
			return rtrn;

		status = static_cast<HttpObj::HttpBodyStatus>(rtrn);
		if (status == HttpObj::CLOSED) // eof found
			return 400;

		if (status == HttpObj::DOING) {

			// check that that part is correct: first line = delim?
			if (current->getFirst() != delim)
				return 400;

			// check next 2 char // [\r\n + DELIM + ".."]
			rtrn = current->tool_check_next_two_char(fd_original);
			if (rtrn >= 100)
				return rtrn;
	
			if (rtrn == HttpObj::DOING) {
				body_parts.push_back(HttpMultipart(*current));
				current = &body_parts.back();
				status = current->getStatus();
				continue;
			}
			// rtrn == HttpObj::CLOSED
			break;
		}
	}
	temp_file& this_file = _request.getFile();

// treat each in vector
	for (std::vector<HttpMultipart>::iterator it = body_parts.begin(); it != body_parts.end(); ++it) {
		// truncate name for '/../../user/psswrd'
		this_file <= it->getFile();
		size_t pos = it->getFilename().find_last_of("/\\");
		it->setFileNName(it->getFilename().substr((pos == std::string::npos) ? 0 : pos + 1));

		if (!it->getFilename().empty()) {
			size_t pos = ressource.find_last_of("/");
			std::string file_to_upload = ressource.substr(0, pos + 1) + it->getFilename();
			
			int rtrn = Method::handleRessource(file_to_upload, query);
			if (rtrn >= 100)
				return rtrn;
		}
		// else: treat metadata (ignored now)
	}
	body_parts.back().getFile().closeTemp(true);
	_answer.setFirstLine(201);
	return Connection::SENDING;
}

/* other "Content-Type":
application/x-www-form-urlencoded
	username=kali&age=42&debug=true

application/json
	{"user":"kali","admin":true}

text/plain
	hello server

application/octet-stream
	binary
 */