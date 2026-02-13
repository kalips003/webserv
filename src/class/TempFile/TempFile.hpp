#ifndef TEMPFILE_HPP
#define TEMPFILE_HPP

#include "Log.hpp"
#include <string>
#include <sys/stat.h>

#define MAX_TEMP_FILES_ALLOWED	1000

///////////////////////////////////////////////////////////////////////////////]
class temp_file {

	public:
///////////////////////////////////////////////////////////////////////////////]
	int				_fd;
	std::string		_path;
	struct stat		_info;
	bool			_delete;
///////////////////////////////////////////////////////////////////////////////]

	temp_file() : _fd(-1), _info(), _delete(true) {}
	~temp_file();

	temp_file& operator=(const temp_file& other);
	temp_file& operator<=(temp_file& other);

	bool		createTempFile(const std::string* root_path);
	bool		openFile(const std::string& path, int flags, bool tmp);
	bool		write(const std::string& s);
	void		closeTemp(bool _delete);

///////////////////////////////////////////////////////////////////////////////]
	/***  GETTERS  ***/
public:
	ssize_t	getBodySize();

///////////////////////////////////////////////////////////////////////////////]
	/***  SETTERS  ***/
public:
	bool	updateFlags(int flags_to_remove, int flags_to_add);
	bool	updateStat();
	void	resetFileFdBegining();

	/***  FRIENDS  ***/
	friend std::ostream& operator<<(std::ostream& os, const temp_file& r);

};

std::ostream& operator<<(std::ostream& os, const temp_file& r);

#endif
