#include "Server.hpp"
#include "Log.hpp"
#include <unistd.h>
#include <stdlib.h>
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
int main(int ac, char** av)
{
	if (ac != 2) {
		std::cerr << C_430 "require one .conf file as argument" RESET << std::endl;
		return 0;
	}

	Log& logger = Log::instance();
	if (!logger.getStatus()) {
		std::cerr << RED "Log failed to setup" RESET << std::endl;
		return 0;
	}

	Server  server(av[1]);
	if (!server.getStatus())
		return 0;

	server.run();

	return 0;
}
