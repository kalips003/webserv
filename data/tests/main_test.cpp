
#include "../../inc/_colors.h"
#include <string>
#include <iostream>

#include "Log.hpp"
#include "Server.hpp"
#include "Tools1.hpp"
#include "HttpObj.hpp"
#include "Multipart.hpp"

#include "Log.hpp"
#include "Tools1.hpp"
#include "Tools2.hpp"
#include "Ft_Post.hpp"
#include <unistd.h>
#include <fcntl.h>
///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
int main(int ac, char** av)
{
	std::string file_path = "www/web_cat/donations/file2";
	std::string file_path_abs = "/home/agallon/Documents/webserv/www/web_cat/donations/file2";

	char buffer[4096];
	Connection c(buffer, sizeof(buffer));
	if(c.getRequest().getFile().openFile(file_path_abs.c_str(), O_RDWR, true)) {
		std::cout << "filename: " << c.getRequest().getFile()._path << std::endl;
	}

	Ft_Post	method(c.getTransferData());
	
	std::string delim = "------geckoformboundary36fbe25c27c93e88337fb3cd92fcb5f9";


	HttpRequest& _request = c.getRequest();
	block b;
	b.data.root = "/home/agallon/Documents/webserv/www/web_cat/donations";
	b.data.autoindex = false;
	b.data.post_policy = "replace";



//////////////////////////////
//////////////////////////////
	_request.getFile().resetFileFd();
	_request.getFile().updateStat();
	HttpMultipart onePartMan(delim, "");
	onePartMan.setBytesTotal(_request.getFile()._info.st_size);

	int fd = _request.getFile()._fd;
	HttpObj::HttpBodyStatus status;
	int i = 0;
	while ((status = onePartMan.getStatus()) != HttpObj::DOING) {
		// std::cout << "loop n." << i << std::endl;
		// i++;
		sleep(1);
		int rtrn = onePartMan.parse_multifile(c.getTransferData()._buffer, c.getTransferData()._sizeofbuff, fd);

		if (status == HttpObj::CLOSED) {// eof found
			std::cout << "CLOSED EOF" << std::endl;
			return 400;
		}
		if (status >= 100) {
			std::cout << "bad status: " << status << std::endl;
			return status;
		
		}
		
	}
//////////////////////////////
	std::string& leftover = onePartMan.getLeftovers();
	// check _first == delim
	if (onePartMan.getFirst() != delim) {
		std::cout << "error getFirst" << std::endl;
		return 400;
	}
	// next 2 char == "--"
	if (leftover.size() < delim.size() + 2) {
		std::cout << "error end delim too short" << std::endl;
		return 400;
	}
	if (leftover[delim.size()] != '-' && leftover[delim.size() + 1] != '-') {
		std::cout << "error end delim" << std::endl;
		return 400;
	}
	// check headers for filename?
	if (!onePartMan.getFilename().empty()) {
		size_t pos = file_path_abs.find_last_of("/");

		// /root_path/web_cat/donations/image.ico
		std::string file_to_upload = file_path_abs.substr(0, pos + 1) + onePartMan.getFilename();
		
		std::string file_path_abs;	
		std::string query;
		std::string sanitized = file_path_abs;		

		method.setlocationBlock(&b);
		int rtrn = method.handleFile(file_path_abs, query, sanitized);
		// rename(onePartMan.getFilename().empty());
	}

	std::cout << "hello, finished\n";























	return 0;
}

///////////////////////////////////////////////////////////////////////////////]
// 			FORK
///////////////////////////////////////////////////////////////////////////////]
// // #include <cstdlib> or <stdlib.h> // exit(c++ version)
// // #include <errno.h> // errno
// #include <stdio.h> // fork, perror
// #include <sys/wait.h> // getpid, waitpid, _exit(c version)
// int main() {
//     pid_t pid = fork();

//     if (pid == -1) {
//         perror("fork failed");
//         return 1;
//     }

//     if (pid == 0) {
//         // Child process
//         std::cout << "Child process (PID: " << getpid() << ")\n";
//         _exit(0);
//     } else {
//         // Parent process
//         std::cout << "Parent process (PID: " << getpid() << "), child PID: " << pid << "\n";
//         waitpid(pid, NULL, 0); // wait for child to finish
//     }

//     return 0;
// }

///////////////////////////////////////////////////////////////////////////////]
// 			EXECVE
///////////////////////////////////////////////////////////////////////////////]
// #include <stdio.h> // perror
// #include <unistd.h>
// int main() {
//     char *args[] = {(char*)"/bin/ls", (char*)"-l", NULL};
//     char *env[] = {NULL};

//     std::cout << "Executing /bin/ls\n";
//     if (execve("/bin/ls", args, env) == -1) {
//         perror("execve failed");
//         return 1;
//     }

//     return 0; // never reached if execve succeeds
// }

///////////////////////////////////////////////////////////////////////////////]
// 			KILL
///////////////////////////////////////////////////////////////////////////////]
// #include <stdio.h> // perror
// #include <csignal>
// int main() {
//     pid_t pid = fork();

//     if (pid == 0) {
//         // child process
//         std::cout << "Child sleeping...\n";
//         sleep(10);
//         std::cout << "Child done\n";
//     } else if (pid > 0) {
//         // parent process
//         std::cout << "Parent sending SIGKILL to child\n";
//         kill(pid, SIGKILL);
//     } else {
//         perror("fork failed");
//         return 1;
//     }

//     return 0;
// }

///////////////////////////////////////////////////////////////////////////////]
// 			SIGNAL
///////////////////////////////////////////////////////////////////////////////]
// #include <csignal>
// #include <unistd.h>
// void handler(int signum) {
//     std::cout << "Caught signal " << signum << "\n";
// }

// int main() {
//     // Register handler for SIGINT
//     signal(SIGINT, handler);

//     std::cout << "Press Ctrl+C to trigger signal\n";
//     for (int i = 0; i < 10; ++i) {
//         sleep(1);
//         std::cout << i << " seconds passed\n";
//     }
//     return 0;
// }

///////////////////////////////////////////////////////////////////////////////]
// 			SOCKET: socket
///////////////////////////////////////////////////////////////////////////////]
// #include <stdio.h> // perror
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// int main() {
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd == -1) {
//         perror("socket failed");
//         return 1;
//     }

//     std::cout << "Socket created: FD " << sockfd << std::endl;
//     close(sockfd);
//     return 0;
// }

///////////////////////////////////////////////////////////////////////////////]
// 			SOCKET: bind
///////////////////////////////////////////////////////////////////////////////]
// #include <stdio.h> // perror
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <cstring>
// int main() {
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);

//     struct sockaddr_in addr;
//     std::memset(&addr, 0, sizeof(addr));
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8080);
//     addr.sin_addr.s_addr = INADDR_ANY;

//     if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
//         perror("bind failed");
//         return 1;
//     }

//     std::cout << "Socket bound to port 8080\n";
//     close(sockfd);
//     return 0;
// }

///////////////////////////////////////////////////////////////////////////////]
// 			SOCKET: listen
///////////////////////////////////////////////////////////////////////////////]
// #include <stdio.h> // perror
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <cstring>
// int main() {
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);

//     struct sockaddr_in addr;
//     std::memset(&addr, 0, sizeof(addr));
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8080);
//     addr.sin_addr.s_addr = INADDR_ANY;

//     bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));

//     if (listen(sockfd, 5) == -1) {
//         perror("listen failed");
//         return 1;
//     }

//     std::cout << "Listening on port 8080\n";
//     close(sockfd);
//     return 0;
// }


///////////////////////////////////////////////////////////////////////////////]
// 			SOCKET: accept
///////////////////////////////////////////////////////////////////////////////]
// #include <stdio.h> // perror
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <cstring>
// int main() {
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);

//     struct sockaddr_in addr;
//     std::memset(&addr, 0, sizeof(addr));
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8080);
//     addr.sin_addr.s_addr = INADDR_ANY;

//     bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
//     listen(sockfd, 5);

//     std::cout << "Waiting for client...\n";
//     int client_fd = accept(sockfd, nullptr, nullptr);
//     if (client_fd == -1) {
//         perror("accept failed");
//         return 1;
//     }

//     std::cout << "Client connected: FD " << client_fd << std::endl;
//     close(client_fd);
//     close(sockfd);
//     return 0;
// }


///////////////////////////////////////////////////////////////////////////////]
// 			SOCKET: connect
///////////////////////////////////////////////////////////////////////////////]
// #include <stdio.h> // perror
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <cstring>
// int main() {
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);

//     struct sockaddr_in server_addr;
//     std::memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(8080);
//     inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

//     if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
//         perror("connect failed");
//         return 1;
//     }

//     std::cout << "Connected to server\n";
//     close(sockfd);
//     return 0;
// }
