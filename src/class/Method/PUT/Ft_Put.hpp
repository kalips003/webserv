#ifndef FT_PUT_HPP
#define FT_PUT_HPP

#include "Ft_Post.hpp"

///////////////////////////////////////////////////////////////////////////////]
class   Ft_Put : public Ft_Post {

private:

public:
	Ft_Put(const t_connec_data& data) : Ft_Post(data) {}

// parent virtual funcitons:
	virtual void	printHello();
	virtual int		handleDir(std::string& ressource);
	virtual int		treatContentType(std::string& ressource, std::string& query);

};

#endif