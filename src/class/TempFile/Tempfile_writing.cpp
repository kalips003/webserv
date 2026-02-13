#include "HttpObj.hpp"

#include <fcntl.h>
#include <cerrno>

///////////////////////////////////////////////////////////////////////////////]
/** @brief Update temp_file with a unique temporary file.
 *
 * @param root_path           Pointer to a string specifying the directory where
 *  the temp file should be created. Checked for NULL, but should be valid
 *
 * @return true on success, false + ErrMsg on error.
 * @note - Created with flags: O_RDWR , O_CLOEXEC and permissions 0666
 * @note - Attempts up to MAX_TEMP_FILES_ALLOWED unique filenames before failing.	---*/
bool	temp_file::createTempFile(const std::string* root_path) {

	if (!root_path || _fd != -1)
		return false;

	if (access(root_path->c_str(), W_OK | X_OK) != 0) {
		LOG_ERROR("createTempFile(): Issue with temp directory (" << root_path << ")");
		return false;
	}

	std::string file_begin = *root_path + "/webserv_tmp_";
	std::string temp_name;
	int fd;

	oss num;
	for (int i = 0; i < MAX_TEMP_FILES_ALLOWED; i++) {

		num.str(""); num.clear();
		num << i;
		temp_name = file_begin + num.str();

// FLAGS can be changed later with fcntl()
	// O_EXCL: if file already exist, fail
	// O_CLOEXEC → guarantees you don’t race another process/thread
		if ((fd = open(temp_name.c_str(), O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0666)) >= 0) {

			_path = temp_name;
			_fd = fd;
			return true;
		}
		else if (errno != EEXIST) {
			LOG_ERROR("createTempFile(): open()");
			return false;
		}

	}
	LOG_WARNING("Too many attempts at creating temp file failed");
	return false;
}

/////////////////////////////////////////////////////////////////////////////]
/** Opens a file at the given path with the O_CLOEXEC + specified flags. 
 * (O_RDWR | O_CREAT | O_EXCL, 0666)
 * If tmp is true, the file will be marked for deletion on close.
 *
 * Returns:
 *  - true  : file successfully opened
 *  - false : path empty, file already open, or open() failed  ---*/
bool	temp_file::openFile(const std::string& path, int flags, bool tmp) {

	if (path.empty() || _fd != -1)
		return false;

	int fd;
	if ((fd = open(path.c_str(), O_CLOEXEC | flags, 0666)) >= 0) {

		_path = path;
		_fd = fd;
		_delete = tmp;
		return true;
	}
	
	LOG_ERROR("createTempFile(): open()");
	return false;
}


///////////////////////////////////////////////////////////////////////////////]
/** Closes the file descriptor and optionally deletes the file. */
void	temp_file::closeTemp(bool del) {

	if (del && !_path.empty())
		unlink(_path.c_str());
	_path.clear();

	if (_fd >= 0) {
		close (_fd);
		_fd = -1;	
	}
}

///////////////////////////////////////////////////////////////////////////////]
bool	temp_file::write(const std::string& s) {

	ssize_t rtrn_write;
	if ((rtrn_write = ::write(_fd, s.c_str(), s.size())) < static_cast<ssize_t>(s.size())) {
		LOG_ERROR_SYS("temp_file::write(): partial write");
		return false;
	}
	return true;
}
