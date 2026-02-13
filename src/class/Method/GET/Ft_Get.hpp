#ifndef FT_GET_HPP
#define FT_GET_HPP

#include "Method.hpp"

///////////////////////////////////////////////////////////////////////////////]
class   Ft_Get : public Method {

public:
	Ft_Get(const t_connec_data& data) : Method(data) {}

// parent virtual funcitons:
	virtual void	printHello();
	virtual int		howToHandleFileNotExist(const std::string& ressource, int rtrn_open);
	virtual int		handleFileExist(std::string& ressource);
	virtual int		handleDir(std::string& ressource);
	virtual void	prepareChild(const std::string& ressource, const std::string& query);

private:
    int 			serveAutoIndexing(const std::string& path);

};

#endif