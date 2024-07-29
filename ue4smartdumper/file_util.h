#pragma once
#include <iostream>
#include <Windows.h>

#include <vector>

namespace util {
	void create_and_write_to_file(std::string path, std::string content);
	void create_and_write_to_file(std::string path, BYTE* bytes, size_t size);


	std::string create_vcxproj_file(std::string path,  std::string project_name);
	std::string create_sln_file(std::string path, const std::string& project_name);
}