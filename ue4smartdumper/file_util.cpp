#include "file_util.h"
#include <fstream>
#include <filesystem>
void util::create_and_write_to_file(std::string path, std::string content)
{

	if (std::filesystem::exists(path))
		std::filesystem::remove(path);
	std::ofstream of;

	of.open(path);

	if (of.is_open()) {
		of << content;
		of.close();
	}
}

void util::create_and_write_to_file(std::string path, BYTE* bytes, size_t size)
{
	if (std::filesystem::exists(path))
		std::filesystem::remove(path);
	std::ofstream of;

	of.open(path);

	if (of.is_open()) {
		of.write(reinterpret_cast<char*>(bytes), size);
		of.close();
	}
}

std::string util::create_vcxproj_file(std::string path,  std::string project_name)
{
    std::string vcxproj_content;

    vcxproj_content += R"(<?xml version="1.0" encoding="utf-8"?>)" "\n";
    vcxproj_content += R"(<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">)" "\n";
    vcxproj_content += R"(  <ItemGroup Label="ProjectConfigurations">)" "\n";
    vcxproj_content += R"(    <ProjectConfiguration Include="Debug|x64">)" "\n";
    vcxproj_content += R"(      <Configuration>Debug</Configuration>)" "\n";
    vcxproj_content += R"(      <Platform>x64</Platform>)" "\n";
    vcxproj_content += R"(    </ProjectConfiguration>)" "\n";
    vcxproj_content += R"(    <ProjectConfiguration Include="Release|x64">)" "\n";
    vcxproj_content += R"(      <Configuration>Release</Configuration>)" "\n";
    vcxproj_content += R"(      <Platform>x64</Platform>)" "\n";
    vcxproj_content += R"(    </ProjectConfiguration>)" "\n";
    vcxproj_content += R"(  </ItemGroup>)" "\n";

    vcxproj_content += R"(  <ItemGroup>)" "\n";
    vcxproj_content += R"(    <ClCompile Include="AutoUpdater.cpp" />)" "\n";
    vcxproj_content += R"(    <ClCompile Include="dllmain.cpp" />)" "\n";
    vcxproj_content += R"(  </ItemGroup>)" "\n";

    vcxproj_content += R"(  <ItemGroup>)" "\n";
    vcxproj_content += R"(    <ClInclude Include="DumperStructs.h" />)" "\n";
    vcxproj_content += R"(    <ClInclude Include="AutoUpdater.h" />)" "\n";
    vcxproj_content += R"(  </ItemGroup>)" "\n";

    vcxproj_content += R"(</Project>)" "\n";


    create_and_write_to_file(path + "/" + project_name + ".vcxproj", vcxproj_content);
  

   
}

std::string util::create_sln_file(std::string path, const std::string& project_name)
{
    std::string project_guid = "B3A9D9F1-1BA1-4A6B-B828-6C9A7B31D98D";
    std::ofstream slnFile(path + "\\" + project_name + ".sln");

    if (slnFile.is_open()) {
        slnFile << R"(Microsoft Visual Studio Solution File, Format Version 12.00)" << std::endl;
        slnFile << R"(# Visual Studio Version 16)" << std::endl;
        slnFile << R"(VisualStudioVersion = 16.0.28729.10)" << std::endl;
        slnFile << R"(MinimumVisualStudioVersion = 10.0.40219.1)" << std::endl;
        slnFile << R"(Project("{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}") = ")" << project_name << R"(", ")" << project_name << R"(.vcxproj", "{)" << project_guid << R"(}")" << std::endl;
        slnFile << R"(EndProject)" << std::endl;
        slnFile << R"(Global)" << std::endl;
        slnFile << R"(  GlobalSection(SolutionConfigurationPlatforms) = preSolution)" << std::endl;
        slnFile << R"(    Debug|x64 = Debug|x64)" << std::endl;
        slnFile << R"(    Release|x64 = Release|x64)" << std::endl;
        slnFile << R"(  EndGlobalSection)" << std::endl;
        slnFile << R"(  GlobalSection(ProjectConfigurationPlatforms) = postSolution)" << std::endl;
        slnFile << R"(    {)" << project_guid << R"(}.Debug|x64.ActiveCfg = Debug|x64)" << std::endl;
        slnFile << R"(    {)" << project_guid << R"(}.Debug|x64.Build.0 = Debug|x64)" << std::endl;
        slnFile << R"(    {)" << project_guid << R"(}.Release|x64.ActiveCfg = Release|x64)" << std::endl;
        slnFile << R"(    {)" << project_guid << R"(}.Release|x64.Build.0 = Release|x64)" << std::endl;
        slnFile << R"(  EndGlobalSection)" << std::endl;
        slnFile << R"(EndGlobal)" << std::endl;

        slnFile.close();
    }
}
