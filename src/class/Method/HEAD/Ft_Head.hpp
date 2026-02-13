#ifndef FT_HEAD_HPP
#define FT_HEAD_HPP

#include "Ft_Get.hpp"

///////////////////////////////////////////////////////////////////////////////]
class   Ft_Head : public Ft_Get {

public:
	Ft_Head(const t_connec_data& data) : Ft_Get(data) {}

// parent virtual funcitons:
	virtual void	printHello();

};

#endif
