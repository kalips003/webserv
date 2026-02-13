#include "Ft_Get.hpp"

#include "HttpRequest.hpp"
#include "HttpAnswer.hpp"
#include "Log.hpp"
#include "Tools1.hpp"

#include <dirent.h>
///////////////////////////////////////////////////////////////////////////////]
/** Generate a listing of requested directory, into _answer
 *
 * 
 *
 * @return      ---*/
int Ft_Get::serveAutoIndexing(const std::string& path) {

	DIR* dir = opendir(path.c_str());
	if (!dir) {
		LOG_ERROR(path.c_str());
		return 403;
	}

	std::string listing_html = "<html><body><h1>Index of " + getRequest().getPath() + "</h1>";

	struct dirent* entry;
	listing_html += "<ul>";
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		
		// skip "." and maybe ".." if you want
		if (name == ".") continue;
		if (name == "..") continue;

		std::string base = getRequest().getPath();
		if (base.empty() || base[base.size() - 1] != '/')
			base += '/';
		
		listing_html += "<li><a href=\"" + base + name + "\">" + name + "</a></li>";
	}

	listing_html += "</ul></body></html>";
	getAnswer().setStringBody(listing_html);
	getAnswer().setMIMEtype(".html");

	closedir(dir);

	return Connection::SENDING;
}
