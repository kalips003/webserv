#include <stdio.h> // perror
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);              // server port
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return 1;
    }

    std::string input;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) break;   // Ctrl-D ends the loop

        if (send(sockfd, input.c_str(), input.length(), 0) == -1) {
            perror("send");
            break;
        }

        char buffer[1024];
        ssize_t received = recv(sockfd, buffer, sizeof(buffer)-1, 0);
        if (received > 0) {
            buffer[received] = '\0';
            std::cout << "Server: " << buffer << std::endl;
        }
    }

    close(sockfd);
    return 0;
}
