#include "AutoUpdater.h"
#include "DumperStructs.h"
#include <object_array.h>
#include <map>


namespace SDK
{
	std::map<uint32_t, std::vector<uobject*>> Packages;
	std::map<std::string, UnrealClass*> DumpedClasses;

	uint64_t GetOffset(std::string ClassName, std::string VariableName)
	{
		return DumpedClasses.find(ClassName) != DumpedClasses.end() ?
			(DumpedClasses[ClassName]->Offsets.find(VariableName) != DumpedClasses[ClassName]->Offsets.end()
				? DumpedClasses[ClassName]->Offsets[VariableName] : 0)
			: 0;
	}

	void ProcessStructure(uobject BaseClass)
	{
		auto CastStruct = BaseClass.cast<ustruct>();
		UnrealClass* Class = new UnrealClass(BaseClass);

		Class->Size = BaseClass.cast<ustruct>().get_struct_size();
		Class->CppName = BaseClass.get_cpp_name();
		if (!Class->Size) return;

		for (auto prop : CastStruct.get_properties()) {
			if (!prop.get_address()) break;
			printf("[%s] [%s] [%x] \n", Class->CppName.c_str(), prop.get_fname().to_string().c_str(), prop.cast<uproperty>().get_offset());
			Class->Offsets[prop.get_fname().to_string()] = prop.cast<uproperty>().get_offset();
		}
	


		DumpedClasses[Class->CppName] = Class;
	}
	void Init()
	{
		offsets::init_engine();

		for (auto obj : object_array()) {
			if (!obj) continue;

			auto object = new uobject(obj);


			if (object->isa(uclass::static_class().cast<uclass>()))
			{
				Packages[object->get_package_index()].push_back(object);
			}
		}


		for (auto PackageTree : Packages)
		{
			auto PackageObject = new uobject(object_array::get_by_index(PackageTree.first));
			auto Package = new UnrealPackage(PackageObject, PackageTree.second);

			for (auto Object : Package->Objects) {
				printf("%s \n", Object->get_cpp_name().c_str());
				ProcessStructure(*Object);
			}
		}

		auto offset = GetOffset("UWorld", "PersistentLevel");
		
		printf("Offset %llx \n", offset);
	}
}