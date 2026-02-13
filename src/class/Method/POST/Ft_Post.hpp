#ifndef FT_POST_HPP
#define FT_POST_HPP

#include "Method.hpp"

///////////////////////////////////////////////////////////////////////////////]
class   Ft_Post : public Method {

public:
	Ft_Post(const t_connec_data& data) : Method(data) {}

// parent virtual funcitons:
	virtual void	printHello();
	virtual int		howToHandleFileNotExist(const std::string& ressource, int rtrn_open);
	virtual int		handleFileExist(std::string& ressource);
	virtual int		handleDir(std::string& ressource);
	virtual void	prepareChild(const std::string& ressource, const std::string& query);
	virtual int		appendFile(const std::string& path);
	virtual int		treatContentType(std::string& ressource, std::string& query);

private:
	int				treatMultipart(std::string& delim, std::string& ressource, std::string& query);

};

#endif
