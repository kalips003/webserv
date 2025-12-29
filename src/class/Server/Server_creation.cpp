#include "Server.hpp"

#include <iostream>

#include "T_tools.cpp"

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
// Constructor for the Server.
// if the constructor finishes and _server_status == OK
    // > the server is listening and ready for accept() / epoll()
#include <cstring> //memset
Server::Server( const char* confi_file ) : _addr(), _socket_fd(-1), _server_status(false) {

	_server_status = _settings.parse_config_file(confi_file);
	if (!_server_status)
		return ;

	_server_status = _settings.check_settings();
	if (!_server_status)
		return ;

	_server_status = create_listening_socket(*this);
	if (!_server_status)
		return ;

	_server_status = true;
	std::cout << C_151 "Server up and running on port: " RESET << _settings._port_num << std::endl;
}


