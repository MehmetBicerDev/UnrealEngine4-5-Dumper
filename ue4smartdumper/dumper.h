#pragma once
#include <iostream>



namespace dumper {
	extern std::string game_name;
	extern std::string game_name_without_version;
	void init();

	void dump();
}