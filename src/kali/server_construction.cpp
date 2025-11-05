#include "server.hpp"

#include <unistd.h>

#include "defines.hpp"

///////////////////////////////////////////////////////////////////////////////]
bool    check_settings(server_settings& settings);
bool    atoi_v2(std::string& input, int& rtrn);
bool    printErr(const char* errmsg);

///////////////////////////////////////////////////////////////////////////////]
#include <cstring> //memset
Server::Server( const char* confi_file ) : _addr(), _socket_fd(-1), _server_status(false) {

    _server_status = parse_config_file(confi_file, _settings);
    if (!_server_status)
        return ;

    _server_status = check_settings(_settings);
    if (!_server_status)
        return ;

    _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket_fd < 0) {
        _server_status = printErr(RED "socket() failed" RESET);
        return ;
    }
    _addr.sin_port = htons(_settings.port_num);
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY; //  or: inet_addr("192.168.1.100");

    // in case of rapid on/off of the server (TIME_WAIT state), avoid the EADDRINUSE bind error
    int opt = 1;
    setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int bind_status = bind(_socket_fd, (struct sockaddr *)&_addr, sizeof(_addr));
    if (!!bind_status) {
    // most common because port already in use: EADDRINUSE
    // or wrong IPaddr, or permission issue: EACCES (port < 1024 = privileged port)
        _server_status = printErr(RED "bind() failed" RESET);
        return ;
    }

// listen() marks the socket as ready to recieve
// connect() marks the socket as ready to send/initiate
    // how many client can tried to connect to this socket while i call accept()
    int listen_status = listen(_socket_fd, HOW_MANY_REQUEST_PER_LISTEN);
    // 3 step handshake: SYN > SYN-ACK > ACK
    if (!!listen_status) {
        _server_status = printErr(RED "listen() failed" RESET);
        return ;
    }
    _server_status = true;
}

///////////////////////////////////////////////////////////////////////////////]
Server::~Server() {

    if (_socket_fd >= 0)
        close(_socket_fd);
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
/*
in socket programming, if you do something like:
    addr.sin_port = htons(0);

it tells the kernel:
    “pick any available ephemeral port for me.”

then, after you bind(), you can find out which port was actually chosen with:
    socklen_t len = sizeof(addr);
    getsockname(sock_fd, (struct sockaddr*)&addr, &len);
    std::cout << "bound port: " << ntohs(addr.sin_port) << std::endl;

so in short:
    port == 0 → kernel automatically assigns a free port.
    it’s valid, but not what you usually want for a web server, since clients won’t know which port to connect to.

*/  // >> SO TECHNICALY LISTEN 0 IS POSSIBLE
typedef std::pair<std::string, std::string> kv;
typedef std::map<std::string, std::string> map;
bool    check_settings(server_settings& settings) {

    map defaults;
    defaults["listen"]      = "8080";
    defaults["server_name"] = "myserver.local";
    defaults["root"]        = "/var/www/html";
    defaults["index"]       = "index.html";

    for (map::iterator it = defaults.begin(); it != defaults.end(); ++it) {
        if (settings.global_settings.find(it->first) == settings.global_settings.end()) {
            std::cerr << RED "Necessary setting (" RESET << it->first << RED ") missing from config" RESET << std::endl;
            std::cerr << it->first << C_142 ": set to default (" RESET << it->second << C_142 ")" RESET << std::endl;
            settings.global_settings[it->first] = it->second;
        }
    }
    if (!atoi_v2(settings.global_settings["listen"], settings.port_num) || 
        settings.port_num <= 0 || settings.port_num > 65535) {
            std::cerr << ERR3 "Invalid port number: " << settings.port_num << std::endl;
            return false;
    }
/* CHECK IF WE CAN OPEN THE SETTING DIRECTORIES */
    return true;
}

///////////////////////////////////////////////////////////////////////////////]
#include <climits>
bool    atoi_v2(std::string& input, int& rtrn) {

    char* end = NULL;
    long num = std::strtol(input.c_str(), &end, 10);
    if (*end != '\0' || num > INT_MAX || num < INT_MIN) {
        std::cerr << RED "Input number invalid: " RESET << input << std::endl;
        return false;
    }
    rtrn = static_cast<int>(num);
    return true;
}

///////////////////////////////////////////////////////////////////////////////]
#include <stdio.h> // perror
bool printErr(const char* errmsg) {

    perror(errmsg);
    std::cout << ERR0 << errmsg << std::endl;
    std::cout << C_412 "ERRNO: " RESET << errno << std::endl;
    return false;
}