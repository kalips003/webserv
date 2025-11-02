#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_EVENTS 128

void server() {
  int server_fd =
      socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
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
  if (listen(server_fd, SOMAXCONN) != 0)
    throw std::runtime_error("listen failed: " + std::string(strerror(errno)));

  int epoll_fd = epoll_create(42);
  if (epoll_fd == -1)
    throw std::runtime_error("epoll_create failed: " +
                             std::string(strerror(errno)));

  struct epoll_event ev, events[MAX_EVENTS];
  ev.events = EPOLLIN;
  ev.data.fd = server_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
    throw std::runtime_error("epoll_ctl failed: " +
                             std::string(strerror(errno)));

  while (1) {
    std::cout << "waiting for client" << std::endl;
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (nfds == -1)
      throw std::runtime_error("epoll_wait failed: " +
                               std::string(strerror(errno)));
    for (int n = 0; n < nfds; n++) {
      if (events[n].data.fd == server_fd) {
        struct sockaddr_in client;
        socklen_t s_client = sizeof(client);
        int client_fd = accept(server_fd, (sockaddr *)(&client), &s_client);
        if (client_fd == -1)
          throw std::runtime_error("accept failed: " +
                                   std::string(strerror(errno)));
        std::cout << "Connected to client" << std::endl;
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        ev.events = EPOLLIN;
        ev.data.fd = client_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
          throw std::runtime_error("epoll_ctl failed: " +
                                   std::string(strerror(errno)));
      } else {
        char buff[1024];
        if (events[n].events & EPOLLIN) {
          if (recv(events[n].data.fd, buff, 1024, 0) == -1)
            throw std::runtime_error("recv failed: " +
                                     std::string(strerror(errno)));
          std::cout << "Got: " << std::string(buff) << std::endl;
          ev.events = EPOLLOUT;
          ev.data.fd = events[n].data.fd;
          if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[n].data.fd, &ev) == -1)
            throw std::runtime_error("epoll_ctl failed: " +
                                     std::string(strerror(errno)));
        } else if (events[n].events & EPOLLOUT) {
          std::string out_msg =
              "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
              "14\r\n\r\nHello, World!\n";
          if (send(events[n].data.fd, out_msg.c_str(), out_msg.length(), 0) ==
              -1)
            throw std::runtime_error("send failed: " +
                                     std::string(strerror(errno)));
          ev.events = EPOLLIN;
          ev.data.fd = events[n].data.fd;
          if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[n].data.fd, &ev) == -1)
            throw std::runtime_error("epoll_ctl failed: " +
                                     std::string(strerror(errno)));
        }
      }
    }
  }
}
