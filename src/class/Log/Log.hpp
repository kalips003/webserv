#ifndef LOG_HPP
#define LOG_HPP

#include "_colors.h"

// LogLevels.hpp
#define LVL_NONE	0
#define LVL_ERROR_SYSTEM	(1 << 0)  // 0b000001
#define LVL_ERROR			(1 << 1)  // 0b000010
#define LVL_WARNING			(1 << 2)  // 0b000100
#define LVL_INFO			(1 << 3)  // 0b001000
#define LVL_DEBUG			(1 << 4)  // 0b010000
#define LVL_LOG				(1 << 5)  // 0b100000

// LVL_ERROR_SYSTEM | LVL_ERROR | LVL_WARNING | LVL_INFO | LVL_DEBUG | LVL_LOG)
// LOGGING OUTPUT
#ifndef LOG_LEVEL
# define LOG_LEVEL ( LVL_ERROR_SYSTEM | LVL_ERROR | LVL_DEBUG | LVL_LOG)
#endif

// TERMINAL OUTPUT
#ifndef PRINT_LEVEL
# define PRINT_LEVEL ( LVL_ERROR_SYSTEM | LVL_ERROR | LVL_WARNING | LVL_INFO)
#endif


#define LOG_PATH "log/"

// LOG STAMP FOR TERMINAL OUTPUT 
#define ERROR_SYS_P	"[ " C_510 "- SYS ERROR -" RESET " ] "
#define ERROR_P		"[" C_510 " ERROR " RESET "] "
#define WARNING_P	"[" C_441 "WARNING" RESET "] "
#define INFO_P		"[" C_150 " INFO  " RESET "] "
#define DEBUG_P		"[" C_124 " DEBUG " RESET "] "
#define LOG_P		"[" C_035 "  L0G  " RESET "] "

#define OTHER_P		"<<<" C_520 " HERE " RESET ">>> "

// LOG STAMP FOR FILE OUTPUT 
#define ERROR_SYS_F	"[ - SYS ERROR - ] "
#define ERROR_F		"[ ERROR ] "
#define WARNING_F	"[WARNING] "
#define INFO_F		"[ INFO  ] "
#define DEBUG_F		"[ DEBUG ] "
#define LOG_F		"[  L0G  ] "

// Log.hpp
#pragma once
#include <sstream>

///////////////////////////////////////////////////////////////////////////////]
class Log {

private:
	int		_fd;  // log file descriptor
	bool	_status;
///////////////////////////////////////////////////////////////////////////////]
/*** Unique instantiation ****/
private:
	Log();
public:
	~Log();
	static Log&	instance() { static Log logger; return logger; }

private:
	void		openFile(const std::string& path);
	bool		createLogging(const std::string& prefix = "webserv_log");
	std::string	removeColors(const std::ostringstream& msg);

//-----------------------------------------------------------------------------]
	/*** LOG FUNCTIONS ***/
public:
	static void	log(const std::string& prefix, const std::string& msg);
	void 	log_here(const std::ostringstream& msg);

	void	log_error_sys(const std::ostringstream& msg);
	void	log_error(const std::ostringstream& msg);
	void	log_warning(const std::ostringstream& msg);
	void	log_info(const std::ostringstream& msg);
	void	log_debug(const std::ostringstream& msg);
	void	log_log(const std::ostringstream& msg);

//-----------------------------------------------------------------------------]
public:
	bool	getStatus() { return _status; }
};

///////////////////////////////////////////////////////////////////////////////]
// 					Macros for compile-time stripping
// usage: LOG_ERROR_MSG("Failed to open file: " << path << ", errno=" << errno);
///////////////////////////////////////////////////////////////////////////////]

#ifndef LOG_HERE
# define LOG_HERE(x) { std::ostringstream oss; oss << x; Log::instance().log_here(oss); }
#endif

// / LVL_ERROR_SYSTEM
#if (PRINT_LEVEL | LOG_LEVEL) & LVL_ERROR_SYSTEM
# define LOG_ERROR_SYS(x) { std::ostringstream oss; oss << x; Log::instance().log_error_sys(oss); }
#else
# define LOG_ERROR_SYS(x) do {} while(0)
#endif

// LOG_ERROR
#if (PRINT_LEVEL | LOG_LEVEL) & LVL_ERROR
# define LOG_ERROR(x) { std::ostringstream oss; oss << x; Log::instance().log_error(oss); }
#else
# define LOG_ERROR(x) do {} while(0)
#endif


// LVL_WARNING
#if (PRINT_LEVEL | LOG_LEVEL) & LVL_WARNING
# define LOG_WARNING(x) { std::ostringstream oss; oss << x; Log::instance().log_warning(oss); }
#else
# define LOG_WARNING(x) do {} while(0)
#endif


// LVL_INFO
#if (PRINT_LEVEL | LOG_LEVEL) & LVL_INFO
# define LOG_INFO(x) { std::ostringstream oss; oss << x; Log::instance().log_info(oss); }
#else
# define LOG_INFO(x) do {} while(0)
#endif


// LVL_DEBUG
#if (PRINT_LEVEL | LOG_LEVEL) & LVL_DEBUG
# define LOG_DEBUG(x) { std::ostringstream oss; oss << x; Log::instance().log_debug(oss); }
#else
# define LOG_DEBUG(x) do {} while(0)
#endif

// LVL_LOG
#if (PRINT_LEVEL | LOG_LEVEL) & LVL_LOG
# define LOG_LOG(x) { std::ostringstream oss; oss << x; Log::instance().log_log(oss); }
#else
# define LOG_LOG(x) do {} while(0)
#endif


#endif
