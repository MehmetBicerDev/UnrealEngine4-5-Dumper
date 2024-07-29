#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <regex>
#include <map>
#include <random>
#include <iomanip>
#include <fstream>
#include <utility>
#include <filesystem>
#include <tuple>
#include <fmt/format.h>
#include <stdexcept>
namespace fs = std::filesystem;



// Function to get filter path from a given path
std::string FilterFromPath(const std::string& path) {
    auto p = fs::path(path);
    auto head = p.parent_path().string();
    auto tail = p.filename().string();

    head = head.replace(head.begin(), head.end(), "\\", "\\\\");
    return head;
}
class UUIDGenerator {
private:
    std::random_device rd;
    std::mt19937_64 gen;

public:
    UUIDGenerator() : gen(rd()) {}

    std::string generateUUID() {
        std::uniform_int_distribution<uint64_t> dis(0, (uint64_t)-1);

        uint64_t rnd1 = dis(gen);
        uint64_t rnd2 = dis(gen);

        // Version 4 UUID (random)
        rnd1 = (rnd1 & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        rnd2 = (rnd2 & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << rnd1 << std::setw(16) << rnd2;
        return ss.str();
    }
};
namespace Vcxproj {
    const std::string Header = R"(<?xml version="1.0" encoding="utf-8"?>)";
    const std::string Project0 = R"(<Project DefaultTargets="Build" ToolsVersion="14.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">)";
    const std::string Project1 = "</Project>";
    const std::string ProjectConfigurations0 = R"(  <ItemGroup Label="ProjectConfigurations">)";
    const std::string ProjectConfigurations1 = R"(  </ItemGroup>)";

    std::string Configuration(const std::string& configuration, const std::string& platform) {
        return fmt::format(R"(
    <ProjectConfiguration Include="{0}|{1}">
      <Configuration>{0}</Configuration>
      <Platform>{1}</Platform>
    </ProjectConfiguration>)", configuration, platform);
    }

    std::string Globals(const std::string& name) {

       
        return fmt::format(R"(
  <PropertyGroup Label="Globals">
    <ProjectGuid>{{{1}}}</ProjectGuid>
    <RootNamespace>{0}</RootNamespace>
  </PropertyGroup>)", name, UUIDGenerator().generateUUID());
    }

    std::string Property(const std::string& config, const std::string& platform) {
        return fmt::format(R"(
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='{0}|{1}'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>{2}</UseDebugLibraries>
    <PlatformToolset>v140</PlatformToolset>
    <CharacterSet>MultiByte</CharacterSet>
  </PropertyGroup>)", config, platform, (config == "Debug" ? "true" : "false"));
    }

     std::string ItemDefenitionDebugT = R"(
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='{0}|{1}'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <Optimization>Disabled</Optimization>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ClCompile>
    <Link>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>)";

     std::string ItemDefenitionReleaseT = R"(
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='{0}|{1}'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <Optimization>MaxSpeed</Optimization>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ClCompile>
    <Link>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
    </Link>
  </ItemDefinitionGroup>)";

    std::string ItemDefenition(std::string configuration, std::string platform) {
        if (configuration == "Debug") {
            return fmt::format(R"(
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='{0}|{1}'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <Optimization>Disabled</Optimization>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ClCompile>
    <Link>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>)", configuration, platform);
        }
        else {
            return fmt::format(R"(
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='{0}|{1}'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <Optimization>MaxSpeed</Optimization>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ClCompile>
    <Link>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
    </Link>
  </ItemDefinitionGroup>)", configuration, platform);
        }
    }

    const std::string ItemGroup0 = R"(  <ItemGroup>)";
    const std::string ItemGroup1 = R"(  </ItemGroup>)";

    std::string Includes(const std::string& name) {
        return fmt::format(R"(
    <ClInclude Include="{0}" />)", name);
    }

    std::string Sources(const std::string& name) {
        return fmt::format(R"(
    <ClCompile Include="{0}" />)", name);
    }

    const std::string ImportTargets = R"(
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />)";

    const std::string ImportProps = R"(
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />)";

    const std::string ImportDefaultProps = R"(
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />)";
}

namespace Filters {
    const std::string Header = R"(<?xml version="1.0" encoding="utf-8"?>)";
    const std::string Project0 = R"(<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">)";
    const std::string Project1 = "</Project>";
    const std::string ItemGroup0 = R"(  <ItemGroup>)";
    const std::string ItemGroup1 = R"(  </ItemGroup>)";

    std::string Sources(const std::string& path) {
        std::string folder = FilterFromPath(path);
        return fmt::format(R"(
    <ClCompile Include="{0}">
      <Filter>{1}</Filter>
    </ClCompile>)", path, folder);
    }

    std::string Includes(const std::string& path) {
        std::string folder = FilterFromPath(path);
        return fmt::format(R"(
    <ClInclude Include="{0}">
      <Filter>{1}</Filter>
    </ClInclude>)", path, folder);
    }

    std::string Folders(const std::string& folder) {
        return fmt::format(R"(
    <Filter Include="{0}">
      <UniqueIdentifier>{{{1}}}</UniqueIdentifier>
    </Filter>)", folder, UUIDGenerator().generateUUID());
    }
}


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class Generator {
private:
    std::vector<std::string> platforms;
    std::vector<std::string> configurations;
    std::vector<std::string> cppFiles;
    std::vector<std::string> headerFiles;

public:
    Generator(const std::string& projectName, const std::vector<std::string>& platforms, const std::vector<std::string>& configurations)
        : platforms(platforms), configurations(configurations) {
        // Optionally, you can set the project name here if needed
    }

    void AddHeader(const std::string& headerFile) {
        headerFiles.push_back(headerFile);
    }

    void AddSource(const std::string& cppFile) {
        cppFiles.push_back(cppFile);
    }

    void Generate(const std::string& projectFileName, const std::string& filtersFileName) {
        std::ofstream projectFile(projectFileName);
        if (!projectFile.is_open()) {
            std::cerr << "Failed to open project file: " << projectFileName << std::endl;
            return;
        }

        std::ofstream filtersFile(filtersFileName);
        if (!filtersFile.is_open()) {
            std::cerr << "Failed to open filters file: " << filtersFileName << std::endl;
            return;
        }

        // Write project file
        WriteProjectFile(projectFile);

        // Write filters file
        WriteFiltersFile(filtersFile);

        std::cout << "Project and filters files generated successfully." << std::endl;
    }

private:
    void WriteProjectFile(std::ofstream& projectFile) {
        projectFile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
        projectFile << "<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << std::endl;

        WriteProjectConfigurations(projectFile);
        WriteGlobals(projectFile);
        WritePropertyGroups(projectFile);
        WriteItemDefinitions(projectFile);
        WriteItemGroups(projectFile);

        projectFile << "  <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />" << std::endl;
        projectFile << "</Project>" << std::endl;
    }

    void WriteProjectConfigurations(std::ofstream& projectFile) {
        projectFile << "  <ItemGroup Label=\"ProjectConfigurations\">" << std::endl;
        for (const auto& configuration : configurations) {
            for (const auto& platform : platforms) {
                projectFile << "    <ProjectConfiguration Include=\"" << configuration << "|" << platform << "\">" << std::endl;
                projectFile << "      <Configuration>" << configuration << "</Configuration>" << std::endl;
                projectFile << "      <Platform>" << platform << "</Platform>" << std::endl;
                projectFile << "    </ProjectConfiguration>" << std::endl;
            }
        }
        projectFile << "  </ItemGroup>" << std::endl;
    }

    void WriteGlobals(std::ofstream& projectFile) {
        projectFile << "  <PropertyGroup Label=\"Globals\">" << std::endl;
        projectFile << "    <ProjectGuid>{YOUR-PROJECT-GUID}</ProjectGuid>" << std::endl; // Replace with actual GUID
        projectFile << "    <RootNamespace>MyProject</RootNamespace>" << std::endl;
        projectFile << "  </PropertyGroup>" << std::endl;
    }

    void WritePropertyGroups(std::ofstream& projectFile) {
        for (const auto& configuration : configurations) {
            for (const auto& platform : platforms) {
                projectFile << "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='" << configuration << "|" << platform << "'\" Label=\"Configuration\">" << std::endl;
                projectFile << "    <ConfigurationType>Application</ConfigurationType>" << std::endl;
                projectFile << "    <UseDebugLibraries>" << (configuration == "Debug" ? "true" : "false") << "</UseDebugLibraries>" << std::endl;
                projectFile << "    <PlatformToolset>v140</PlatformToolset>" << std::endl;
                projectFile << "    <CharacterSet>MultiByte</CharacterSet>" << std::endl;
                projectFile << "  </PropertyGroup>" << std::endl;
            }
        }
    }

    void WriteItemDefinitions(std::ofstream& projectFile) {
        for (const auto& configuration : configurations) {
            for (const auto& platform : platforms) {
                projectFile << "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" << configuration << "|" << platform << "'\">" << std::endl;
                projectFile << "    <ClCompile>" << std::endl;
                projectFile << "      <WarningLevel>Level3</WarningLevel>" << std::endl;
                projectFile << "      <Optimization>Disabled</Optimization>" << std::endl;
                projectFile << "      <SDLCheck>true</SDLCheck>" << std::endl;
                projectFile << "      <PreprocessorDefinitions>%(PreprocessorDefinitions)</PreprocessorDefinitions>" << std::endl;
                projectFile << "    </ClCompile>" << std::endl;
                projectFile << "    <Link>" << std::endl;
                projectFile << "      <GenerateDebugInformation>true</GenerateDebugInformation>" << std::endl;
                projectFile << "    </Link>" << std::endl;
                projectFile << "  </ItemDefinitionGroup>" << std::endl;
            }
        }
    }

    void WriteItemGroups(std::ofstream& projectFile) {
        projectFile << "  <ItemGroup>" << std::endl;
        for (const auto& header : headerFiles) {
            projectFile << "    <ClInclude Include=\"" << header << "\" />" << std::endl;
        }
        projectFile << "  </ItemGroup>" << std::endl;

        projectFile << "  <ItemGroup>" << std::endl;
        for (const auto& cpp : cppFiles) {
            projectFile << "    <ClCompile Include=\"" << cpp << "\" />" << std::endl;
        }
        projectFile << "  </ItemGroup>" << std::endl;
    }

    void WriteFiltersFile(std::ofstream& filtersFile) {
        filtersFile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
        filtersFile << "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << std::endl;

        WriteFilterItemGroups(filtersFile);

        filtersFile << "</Project>" << std::endl;
    }

    void WriteFilterItemGroups(std::ofstream& filtersFile) {
        filtersFile << "  <ItemGroup>" << std::endl;

        for (const auto& header : headerFiles) {
            WriteFilterHeader(filtersFile, header);
        }

        for (const auto& cpp : cppFiles) {
            WriteFilterSource(filtersFile, cpp);
        }

        filtersFile << "  </ItemGroup>" << std::endl;
    }

    void WriteFilterHeader(std::ofstream& filtersFile, const std::string& header) {
        auto folder = FilterFromPath(header);
        filtersFile << "    <ClInclude Include=\"" << header << "\">" << std::endl;
        filtersFile << "      <Filter>" << folder << "</Filter>" << std::endl;
        filtersFile << "    </ClInclude>" << std::endl;
    }

    void WriteFilterSource(std::ofstream& filtersFile, const std::string& cpp) {
        auto folder = FilterFromPath(cpp);
        filtersFile << "    <ClCompile Include=\"" << cpp << "\">" << std::endl;
        filtersFile << "      <Filter>" << folder << "</Filter>" << std::endl;
        filtersFile << "    </ClCompile>" << std::endl;
    }

    std::string FilterFromPath(const std::string& path) {
        auto parent_path = fs::path(path).parent_path();
        if (parent_path == ".")
            return "";
        return parent_path.string();
    }
};

