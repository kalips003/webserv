#include "Task.hpp"

#include "Connection.hpp"
#include "Server.hpp"

///////////////////////////////////////////////////////////////////////////////]
Task::Task(Connection& connec)
 : _request(connec.getRequest()), _answer(connec.getAnswer()), _status(0) {}

///////////////////////////////////////////////////////////////////////////////]
#include "Ft_Get.hpp"
Task* Task::createTask(const std::string& method, Connection& connec) {
    if (method == "GET")       return new Ft_Get(connec);
    // else if (method == "POST") return new Ft_post(req, ans);
    // else if (method == "PUT")  return new Ft_put(req, ans);
    // ... other methods
    else return NULL;  // unknown method → 405 or reject
}

///////////////////////////////////////////////////////////////////////////////]
/** Check whether a requested resource should be handled as a CGI script
 *
 * Determines if the file extension of the given path matches a CGI handler
 * defined in the server configuration.
 *
 * @param path  Requested resource path (without query string)
 * @return      Pointer to the CGI interpreter path if the resource is CGI,
 *              NULL otherwise			---*/
const std::string* Task::isCGI(const std::string& path) const {

	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return NULL;
	std::string extension = path.substr(pos);
	return g_settings.find_setting_in_blocks("cgi", "", extension);
}
