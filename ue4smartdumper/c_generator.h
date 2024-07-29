#pragma once
#include "unreal_struct.h"

namespace dumper {
	class c_generator 
	{
	public:

		void test()
		{

		}

		virtual void generate(std::string path, std::vector<unreal_package*> packages) = 0;
	};
}