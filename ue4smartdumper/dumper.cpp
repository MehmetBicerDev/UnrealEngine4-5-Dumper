#include "dumper.h"
#include "class_manager.h"
#include "unreal_struct.h"
#include <offsets.h>
#include <map>
#include <unreal_objects.h>
#include <object_array.h>

#include "internal_autoupdate_generator.h"
namespace dumper {

	DWORD64 dump_start = 0;
	std::string game_name = "";
	std::string game_name_without_version = "";

	std::map<uint32_t, std::vector<uobject*>> packages;

	std::vector<unreal_package*> dump_output;
	void init()
	{
		offsets::init_engine();
		fstring name;
		fstring version;

		auto kismet = object_array::find_class_fast("KismetSystemLibrary");

		auto get_game_name = kismet.get_function("KismetSystemLibrary", "GetGameName");
		auto get_engine_version = kismet.get_function("KismetSystemLibrary", "GetEngineVersion");

		kismet.process_event(get_game_name, &name);
		kismet.process_event(get_engine_version, &version);

		game_name = name.ToString() + "_" + version.ToString();
		game_name_without_version = name.ToString();

	}	




	void dump()
	{
		dump_start = GetTickCount64();
		for (auto obj : object_array()) {
			if (!obj) continue;

			auto object = new uobject(obj);


			if (object->isa(uclass::static_class().cast<uclass>()) || object->isa(ustruct::static_class().cast<uclass>()) || object->isa(uenum::static_class().cast<uclass>()))
			{
				packages[object->get_package_index()].push_back(object);
			}
		}


		for (auto package_tree : packages)
		{
			auto package_object = new uobject(object_array::get_by_index(package_tree.first));
			auto package = new unreal_package(new upackage(package_object, package_tree.second));

			package->process();
			dump_output.push_back(package);
		}

		internal_autoupdater_generator->generate("C:", dump_output);

		printf("Took %d seconds. \n", (GetTickCount64() - dump_start) / 1000);
	

	}
}