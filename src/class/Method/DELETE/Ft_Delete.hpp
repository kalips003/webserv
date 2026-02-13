#ifndef FT_DELETE_HPP
#define FT_DELETE_HPP

#include "Method.hpp"

///////////////////////////////////////////////////////////////////////////////]
class   Ft_Delete : public Method {

public:
	Ft_Delete(const t_connec_data& data) : Method(data) {}

// parent virtual funcitons:
	virtual void	printHello();
	virtual int		howToHandleFileNotExist(const std::string& ressource, int rtrn_open);
	virtual int		handleFileExist(std::string& ressource);
	virtual int		handleDir(std::string& ressource);
	virtual void	prepareChild(const std::string& ressource, const std::string& query);

};

#endif
