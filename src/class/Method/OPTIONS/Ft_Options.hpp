#ifndef FT_OPTIONS_HPP
#define FT_OPTIONS_HPP

#include "Method.hpp"

///////////////////////////////////////////////////////////////////////////////]
class   Ft_Options : public Method {

private:

public:
	Ft_Options(const t_connec_data& data) : Method(data) {}

// parent virtual funcitons:
	virtual void	printHello();
	virtual int		howToHandleFileNotExist(const std::string& ressource, int rtrn_open);
	virtual int		handleFileExist(std::string& ressource);
	virtual int		handleDir(std::string& ressource);
	virtual void	prepareChild(const std::string& ressource, const std::string& query);

};

#endif
