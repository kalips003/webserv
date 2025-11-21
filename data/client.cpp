#include <stdio.h> // perror
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../inc/_colors.h"

///////////////////////////////////////////////////////////////////////////////]
#include <fcntl.h>
bool    set_flags(int fd_to_set, int flag_to_add) {

    int flags = fcntl(fd_to_set, F_GETFL, 0);
    if (flags < 0){
        std::cerr <<RED "fcntl( F_GETFL ) failed" RESET << std::endl;
        return false;
    }
    if (fcntl(fd_to_set, F_SETFL, flags | flag_to_add) < 0){
        std::cerr <<RED "accept( F_SETFL ) failed" RESET << std::endl;
        return false;
    }
    return true;
}

#define PORT 9999
///////////////////////////////////////////////////////////////////////////////]
int main() {
std::cout << C_314 "HELLO...?" RESET << std::endl;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);              // server port
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return 1;
    }
    set_flags(sockfd, O_NONBLOCK);
    // std::string input = "ahah\r\nheader: fun!\r\n\r\n";  
    std::string input = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
    bool sent = false;
    while (true) {
        
//         std::cout << "> ";
//         if (!std::getline(std::cin, input)) break;   // Ctrl-D ends the loop
// std::cout << C_314 "Input: " RESET << input << C_314 "\n\tsending...?" RESET << std::endl;
        if (!sent) {
            ssize_t s = send(sockfd, input.c_str(), input.length(), 0);
            if (s < 0) {
                perror("send");
                break;
            }
std::cout << C_114 "Message sent, waiting answer..." RESET << std::endl;
            sent = true;
        }
        
        char buffer[4096];
        ssize_t received = recv(sockfd, buffer, sizeof(buffer)-1, 0);
        if (received > 0) {
            buffer[received] = '\0';
            std::cout << C_114 "recv: " RESET << received << C_114 " bytes" RESET << std::endl;
            std::cout << "Server: " << buffer << std::endl;
        } else if (!received) {
std::cout << C_114 "end of answer received...breaking" RESET << std::endl;
            break;
        } else if (received < 0) {
// std::cout << C_114 "-1 received...breaking" RESET << std::endl;
            continue;
        }
    }

std::cout << C_114 "ending..." RESET << std::endl;
    // std::getline(std::cin, input);   // Ctrl-D ends the loop
    close(sockfd);
    return 0;
}
