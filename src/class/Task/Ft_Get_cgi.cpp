#include "Ft_Get.hpp"
#include "_colors.h"

#include <unistd.h>

#include "Tools1.hpp"
///////////////////////////////////////////////////////////////////////////////]
int Ft_Get::handleCGI(const std::string& ressource, const std::string& query, const std::string* CGI_interpreter_path) {
	(void)ressource;
	(void)query;
	(void)CGI_interpreter_path;
	pid_t pid = fork();
	if (pid < 0) {
		printErr("fork()");
		return 500;
	}

	if (pid == 0) {
		// ---- child ----
		// setenv(...)
		// dup2(...) stdin/stdout
		// execve(...)
		_exit(1); // only if execve fails
	
	}

// ---- parent ----
// read from pipe
// waitpid(pid, ...)



	return 0;
}
