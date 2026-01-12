#include "Ft_Get.hpp"
#include "_colors.h"

#include <iostream>
///////////////////////////////////////////////////////////////////////////////]
//-----------------------------------------------------------------------------]
int Ft_Get::ft_do() {
	std::cout << C_431 "IM ALIVE! (GET)" RESET << std::endl;

	std::string path = getRequest().getPath(); // / index.html
	std::string root = g_settings.getRoot();

	path = root + path;
	std::cout << "path: " << path << std::endl;
	// if (path.notexist???) // /var/www/html + / index.html







	return 0;
}