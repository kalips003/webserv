#include "Log.hpp"

#include <cerrno>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////]
/** oss msg; msg << ""; Log::log(OTHER_P, msg.str()); */
void	Log::log(const std::string& prefix, const std::string& msg) {
	std::cout << prefix << msg << std::endl;
}

//-----------------------------------------------------------------------------]
/**	 */
void Log::log_here(const std::ostringstream& msg) {

	std::cerr << OTHER_P << msg.str() << std::endl;
}

///////////////////////////////////////////////////////////////////////////////]
/***						LOG FUNCTIONS					 				***/
///////////////////////////////////////////////////////////////////////////////]

//-----------------------------------------------------------------------------]
/**	 */
void Log::log_error_sys(const std::ostringstream& msg) {
#if LOG_LEVEL & LVL_ERROR_SYSTEM
	std::string s = ERROR_SYS_F + removeColors(msg) + " (" + strerror(errno) + ")\n";
	if (_fd >= 0)
		write(_fd, s.c_str(), s.size());
#endif

#if PRINT_LEVEL & LVL_ERROR_SYSTEM
	std::cerr << ERROR_SYS_P << msg.str() << " (" << strerror(errno) << ")\n";
#endif
	(void)msg;
}

//-----------------------------------------------------------------------------]
void Log::log_error(const std::ostringstream& msg) {
#if LOG_LEVEL & LVL_ERROR
	std::string s = ERROR_F + removeColors(msg) + "\n";
	if (_fd >= 0)
		write(_fd, s.c_str(), s.size());
#endif

#if PRINT_LEVEL & LVL_ERROR
	std::cout << ERROR_P << msg.str() << std::endl;
#endif
	(void)msg;
}


//-----------------------------------------------------------------------------]
void Log::log_warning(const std::ostringstream& msg) {
#if LOG_LEVEL & LVL_WARNING
	std::string s = WARNING_F + removeColors(msg) + "\n";
	if (_fd >= 0)
		write(_fd, s.c_str(), s.size());
#endif

#if PRINT_LEVEL & LVL_WARNING
	std::cout << WARNING_P << msg.str() << std::endl;
#endif
	(void)msg;
}

//-----------------------------------------------------------------------------]
void Log::log_info(const std::ostringstream& msg) {
#if LOG_LEVEL & LVL_INFO
	std::string s = INFO_F + removeColors(msg) + "\n";
	if (_fd >= 0)
		write(_fd, s.c_str(), s.size());
#endif

#if PRINT_LEVEL & LVL_INFO
	std::cout << INFO_P << msg.str() << std::endl;
#endif
	(void)msg;
}

//-----------------------------------------------------------------------------]
void Log::log_debug(const std::ostringstream& msg) {
#if LOG_LEVEL & LVL_DEBUG
	std::string s = DEBUG_F + removeColors(msg) + "\n";
	if (_fd >= 0)
		write(_fd, s.c_str(), s.size());
#endif

#if PRINT_LEVEL & LVL_DEBUG
	std::cout << DEBUG_P << msg.str() << std::endl;
#endif
	(void)msg;
}

//-----------------------------------------------------------------------------]
void Log::log_log(const std::ostringstream& msg) {
#if LOG_LEVEL & LVL_LOG
	std::string s = LOG_F + removeColors(msg) + "\n";
	if (_fd >= 0)
		write(_fd, s.c_str(), s.size());
#endif

#if PRINT_LEVEL & LVL_LOG
	std::cout << LOG_P << msg.str() << std::endl;
#endif
	(void)msg;
}


///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
Log::Log() : _fd(-1), _status(true) {
#if LOG_LEVEL > LVL_NONE
	_status = createLogging();
#endif
}

///////////////////////////////////////////////////////////////////////////////]
Log::~Log() {
	if (_fd >= 0) { close(_fd); _fd = -1; }
}

#include <sys/stat.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdio.h>
///////////////////////////////////////////////////////////////////////////////]
bool Log::createLogging(const std::string& prefix) {

	struct stat st;
// Exist? if not exist → try to create it
	if (stat(LOG_PATH, &st) != 0) {
		if (mkdir(LOG_PATH, 0777) != 0) {
			perror("mkdir(): " LOG_PATH);
			return (_status = false);
		}
	}
// is a directory
	else if (!S_ISDIR(st.st_mode))
		return (_status = false);
// can be accessed (and written to)
	if (access(LOG_PATH, W_OK) != 0) {// even if file exist, might not be readable by server
		perror("access(): " LOG_PATH);
		return (_status = false);
	}

// create time stamp
std::time_t t = std::time(NULL);
	std::tm tm;
	localtime_r(&t, &tm);
// webserv_log_yyyymmjjhhmmss.log
	std::ostringstream date;
	date << LOG_PATH << prefix << "_"
		<< (tm.tm_year + 1900)
		<< std::setw(2) << std::setfill('0') << (tm.tm_mon + 1)
		<< std::setw(2) << std::setfill('0') << tm.tm_mday
		<< "_"
		<< std::setw(2) << std::setfill('0') << tm.tm_hour
		<< std::setw(2) << std::setfill('0') << tm.tm_min
		<< std::setw(2) << std::setfill('0') << tm.tm_sec
		<< ".log";

	openFile(date.str());
	return _status;
}

///////////////////////////////////////////////////////////////////////////////]
void Log::openFile(const std::string& path) {
	_status = true;
	_fd = open(path.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0666);
	if (_fd >= 0) 
		_status = true;
	else
		_status = false;
}

///////////////////////////////////////////////////////////////////////////////]
std::string		Log::removeColors(const std::ostringstream& msg) {
	std::string out = msg.str();

	for (size_t i = 0; i < out.size(); ) {
		if (out[i] == '\e') {
			size_t end = out.find('m', i);
			if (end == std::string::npos)
				break; 
			out.erase(i, end - i + 1);
		}
		else
			++i;
	}
	return out;
}