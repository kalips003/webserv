
#include "../../inc/_colors.h"
#include <string>
#include <iostream>

#include "HttpRequest.hpp"
#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
int main(int ac, char** av)
{
	HttpRequest	h;
	std::string s = "GET /index.html HTTP/1.1\r\nHost: ex\r";
	std::string s3 = "\nbody-size: 25\r\n\r\n";
	int r = h.readingFirstLine(s);
	std::cout << C_154 "rtrn: " RESET << r << std::endl;
	std::cout << C_234 "_method: " RESET << h.getMethod() << std::endl;
	std::cout << C_234 "_path: " RESET << h.getPath() << std::endl;
	std::cout << C_234 "_version: " RESET << h.getVersion() << std::endl;
	std::cout << C_234 "_buffer: " RESET << h.getBuffer() << std::endl;

	h.readingHeaders(s3);
	std::cout << h;

	std::cout << "-----------------------------------\n";
	std::string s2 = "GET /index.html HTTP/1.1\r\nHost: ex\r\n\r\n";

    std::vector<std::string> v;
    v = splitOnDelimitor(s2, "\r\n");
	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {
		std::cout << C_552 "[" RESET << *it << C_552 "]" RESET << std::endl;
	}
	std::cout << "-----------------------------------\n";
	std::cout << "size: " << v.size();

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
