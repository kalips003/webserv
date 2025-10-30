#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

void server() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server;
  int is_reusable = true;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &is_reusable,
                 sizeof(is_reusable)) != 0)
    throw std::runtime_error("setsockopt failed: " +
                             std::string(strerror(errno)));
  server.sin_family = AF_INET;
  server.sin_port = htons(8080);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(server_fd, (sockaddr *)(&server), sizeof(server)) != 0)
    throw std::runtime_error("bind failed: " + std::string(strerror(errno)));
  if (listen(server_fd, 15) != 0)
    throw std::runtime_error("listen failed: " + std::string(strerror(errno)));
  while (1) {
    std::cout << "waiting for client" << std::endl;
    int client_fd;
    struct sockaddr_in client;
    socklen_t s_client = sizeof(client);
    client_fd = accept(server_fd, (sockaddr *)(&client), &s_client);
    if (client_fd == -1)
      throw std::runtime_error("accept failed: " +
                               std::string(strerror(errno)));
    std::cout << "Connected to client" << std::endl;
    char buff[1024];
    if (recv(client_fd, buff, 1024, 0) == -1)
      throw std::runtime_error("recv failed: " + std::string(strerror(errno)));
    std::cout << "Got: " << std::string(buff) << std::endl;
    std::string out_msg =
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
        "14\r\n\r\nHello, World!\n";
    if (send(client_fd, out_msg.c_str(), out_msg.length(), 0) == -1)
      throw std::runtime_error("send failed: " + std::string(strerror(errno)));
    close(client_fd);
  }
}
