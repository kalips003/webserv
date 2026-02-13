#include "Ft_Put.hpp"

///////////////////////////////////////////////////////////////////////////////]
void Ft_Put::printHello() {
	LOG_DEBUG("PUT method called");
}

///////////////////////////////////////////////////////////////////////////////]
/** Implementation of Method::treatContentType (that return -1)
* here to treat "Content-Type" (implemented, only multipart/form-data) 
* this function find content type == multipart/form-data and recover the boundary
* the next one does the splitting and treating of _request._tmp_file
* @return -1 if no multipart, errCode on any error, Connection::SENDING if all goes well */
int		Ft_Put::treatContentType(std::string& ressource, std::string& query) {

	(void)ressource;
	(void)query;

	return -1;
}

///////////////////////////////////////////////////////////////////////////////]
/** */
int		Ft_Put::handleDir(std::string& ressource) {
	(void)ressource;
	return 403; // or 405 with header {Allow: GET, HEAD}
}
