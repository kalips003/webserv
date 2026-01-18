#include "Ft_Delete.hpp"
#include "_colors.h"

#include "Tools1.hpp"
#include "defines.hpp"
#include "ServerSettings.hpp"
#include "HttpRequest.hpp"
#include "HttpAnswer.hpp"

#include <sys/stat.h>
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////]
int Ft_Delete::ft_do() {
	printLog(DEBUG, "DELETE method called", 1);

// add path to root
	std::string ressource;
	int rtrn = getFullPath(ressource, getRequest().getPath());
	if (rtrn)
		return rtrn;
	oss msg; msg << "Full path of the ressource asked to delete: " << ressource;
	printLog(DEBUG, msg.str(), 1);

// check existance
	struct stat ressource_info;
	rtrn = isFileNOK(ressource, ressource_info);
	if (rtrn) {
		printErr("ressource not OK");
		return rtrn;
	}

// is FILE
	if (S_ISREG(ressource_info.st_mode)) {

// also need write + exec right on parent folder
		size_t pos = ressource.find_last_of('/');
		if (pos == std::string::npos)
			return 500; // ???
		std::string parent_dir = ressource.substr(0, pos);
		if (access(parent_dir.c_str(), W_OK | X_OK) != 0) { // need write + exec right on parent folder
			printErr(parent_dir.c_str());
			return 403; 
		}
	}
	else {
		printLog(ERROR, "Ressource requested for deletion isnt a FILE", 1);
		return 409;
	}

	if (unlink(ressource.c_str()) != 0) {
		printErr("unlink()");
		return 500;
	}
	getAnswer().setFirstLine(204); // should return 204 if successful
	return 0;
}
