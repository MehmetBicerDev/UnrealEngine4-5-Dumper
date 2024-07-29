#include "internal_autoupdate_generator.h"
#include "dumper.h"
#include "engine_structs.h"
#include "file_util.h"
#include "string_util.h"
#include "vcxproj_generator.h"
bool dumper::c_internal_autoupdater_generator::is_blocked_type(std::string type, std::string name)
{
	bool result = false;

	if (type.starts_with("struct") || type.find("FMulticast") != std::string::npos || type.find("TWeakObjectPtr") != std::string::npos || type.starts_with("enum") || type.find("TSet") != std::string::npos || type.find("TSoftObjectPtr") != std::string::npos || type.find("TSubclassOf") != std::string::npos || type.find("TMap") != std::string::npos || type.find("TScriptInterface") != std::string::npos)
		result = true;
	return result;
}
void dumper::c_internal_autoupdater_generator::create_engine_files(std::string path)
{
	std::string engine_path = path + "\\Engine";
	std::filesystem::create_directory(engine_path);
	std::filesystem::create_directory(engine_path + "\\Private");
	std::filesystem::create_directory(engine_path + "\\Public");

	util::create_and_write_to_file(engine_path + "\\Public\\name_array.h", (BYTE*)name_array_header, 1630);
	util::create_and_write_to_file(engine_path + "\\Private\\name_array.cpp", (BYTE*)name_array_cpp, 18258);

	util::create_and_write_to_file(engine_path + "\\Public\\object_array.h", (BYTE*)object_array_header, 3639);
	util::create_and_write_to_file(engine_path + "\\Private\\object_array.cpp", (BYTE*)object_array_cpp, 12747);

	util::create_and_write_to_file(engine_path + "\\Public\\offsets.h", (BYTE*)offsets_header, 4208);
	util::create_and_write_to_file(engine_path + "\\Private\\offsets.cpp", (BYTE*)offsets_cpp, 12269);

	util::create_and_write_to_file(engine_path + "\\Public\\unreal_objects.h", (BYTE*)unreal_objects_header, 10043);
	util::create_and_write_to_file(engine_path + "\\Private\\unreal_objects.cpp", (BYTE*)unreal_objects_cpp, 30978);

	util::create_and_write_to_file(engine_path + "\\Public\\unreal_types.h", (BYTE*)unreal_types_header, 3424);
	util::create_and_write_to_file(engine_path + "\\Private\\unreal_types.cpp", (BYTE*)unreal_types_cpp, 5455);

	util::create_and_write_to_file(engine_path + "\\Public\\enums.h", (BYTE*)enums_header, 26790);
	util::create_and_write_to_file(engine_path + "\\Public\\settings.h", (BYTE*)settings_header, 4056);
	util::create_and_write_to_file(engine_path + "\\Public\\ue4lib_includes.h", (BYTE*)ue4lib_includes_header, 82);
	util::create_and_write_to_file(engine_path + "\\Public\\util.h", (BYTE*)util_header, 32259);
	util::create_and_write_to_file(engine_path + "\\Public\\offset_finder.h", (BYTE*)offset_finder_header, 26014);



	util::create_and_write_to_file(path + "\\AutoUpdater.h", (BYTE*)AutoUpdater_header, 137);
	util::create_and_write_to_file(path + "\\AutoUpdater.cpp", (BYTE*)AutoUpdater_cpp, 1930);
	util::create_and_write_to_file(path + "\\DumperStructs.h", (BYTE*)DumperStructs_header, 1031);
	util::create_and_write_to_file(path + "\\dllmain.cpp", (BYTE*)dllmain_cpp, 718);


	Generator generator(game_name_without_version, {"Win32","x64"}, {"Debug", "Release"});

	generator.AddHeader("DumperStructs.h");
	generator.AddHeader("AutoUpdater.h");
	generator.AddSource("AutoUpdater.cpp");
	generator.AddSource("dllmain.cpp");

	generator.Generate(path + "\\" + game_name_without_version + ".vcxproj", path + "\\" + game_name_without_version + ".vcxproj.filters");
//	util::create_vcxproj_file(path, game_name);

}
void dumper::c_internal_autoupdater_generator::create_directories(std::string path)
{
	std::string sdk_path = path + "\\P2Dumper";
	std::string game_path = sdk_path + "\\" + game_name;
	std::string full_path = game_path + "\\SmartSDK";

	std::filesystem::create_directory(sdk_path);
	std::filesystem::create_directory(game_path);
	std::filesystem::create_directory(full_path);
	std::filesystem::create_directory(full_path + "\\SDK");

	create_engine_files(full_path);


}
std::string dumper::c_internal_autoupdater_generator::dump_sdk(std::string package_name, unreal_struct* klass)
{

	std::string file_data = "#pragma once\n";
	std::string function_data = "";
	file_data += R"(#include <offsets.h>)";
	file_data += "\n";
	file_data += R"(#include <unreal_objects.h>)";
	file_data += "\n\n";
	file_data += "namespace " + package_name + "\n{\n";
	file_data += "	namespace " + klass->CppName + "_Offsets\n	{\n";



	function_data += "	class " + klass->CppName + ": public uobject \n	{\n	public:\n";

	for (auto member : klass->Members) {
		if (!member->Name.starts_with("pad_") && member->Name.find("[") == std::string::npos && !is_blocked_type(member->Type, member->Name)) {

			if (member->Type.find("TArray") != std::string::npos)
				member->Type.replace(member->Type.find("TArray"), 6, "tarray");

			if (member->Type.find("FName") != std::string::npos)
				member->Type.replace(member->Type.find("FName"), 5, "fname");

			if (member->Type.find("FString") != std::string::npos)
				member->Type.replace(member->Type.find("FString"), 7, "fstring");

			if (member->Type.find("FVector") != std::string::npos)
				member->Type.replace(member->Type.find("FVector"), 7, "FVectorA");
			file_data += "		static uint64_t " + member->Name + " = " + util::int_to_hex_string(member->Offset) + ";\n";

			function_data += "		" + member->Type + " Get_" + member->Name + "()\n		{\n";
			function_data += "			return *(" + member->Type + "*)((uintptr_t)this + " + klass->CppName + "_Offsets::" + member->Name + ");\n		}\n";
		}
	}

	function_data += "		void CallFunction(std::string function_name, void* params)\n		{\n			auto fn = ufunction::find_function_objects(function_name);\n			process_event(fn.cast<ufunction>(), params);\n		}\n";
	function_data += "      		template<class T>\n		T* CastClass()\n		{\n			return (T*)this;\n		}\n";

	function_data += "		void CallFunction(void* Func, void* Params)\n		{\n";
	function_data += "			auto vtable = *reinterpret_cast<void***>(this);\n";
	function_data += "			reinterpret_cast<void(*)(void*, void*, void*)>(vtable[offsets::process_event::pe_index])(this, Func, Params);\n		}\n";
	file_data += "	}\n\n";

	
	file_data += function_data + "\n	};\n";

	file_data += "}";
	return file_data;
}
void dumper::c_internal_autoupdater_generator::generate(std::string path, std::vector<unreal_package*> packages)
{
	if (!std::filesystem::exists(path)) { printf("SDK Path(%s) does not exist.", path.c_str()); return; }

	create_directories(path);


	for (auto package : packages)
	{
		std::string package_name = package->Owner->Object->get_name();
		std::string package_path = path + "\\P2Dumper\\" + game_name + "\\SmartSDK\\SDK\\" + package_name;
		std::filesystem::create_directory(package_path);


		for (auto klass : package->Classes)
		{
			if (klass.second->CppName == "ULevel")
				klass.second->AddMember(16, offsets::ulevelo::actors, "tarray<Engine::AActor*>", "LevelActors");
			auto file_data = dump_sdk(package_name, klass.second);

			util::create_and_write_to_file(package_path + "\\" + klass.first + ".h", file_data);
		}

	}

}
