#pragma once
#include "ue4lib_includes.h"




namespace util
{
	template<class t>
	std::string int_to_hex_string(t obj);

	



	template<class t>
	std::string int_to_hex_string(t obj)
	{
		std::stringstream stream;
		stream << std::hex << obj;
		return "0x" + stream.str();
	}
}