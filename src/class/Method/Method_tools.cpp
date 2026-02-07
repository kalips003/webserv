#include "Method.hpp"

#include <sys/stat.h>
#include <cerrno>

///////////////////////////////////////////////////////////////////////////////]
/**	Fills the stat struct of the path
*
* @return 0 if all ok, ErrCode else (403 / 404)		*/
int	Method::isFileNOK(std::string path, struct stat& ressource_info) {

	int rtrn = stat(path.c_str(), &ressource_info);
	if (rtrn) {
		if (errno == ENOENT) // ENOENT → file not found → 404
			return 404;
		if (errno == EACCES) // EACCES → permission denied → 403
			return 403;
	}
	return 0;
}
