#pragma once
#include "Engine/Public/ue4lib_includes.h"
#include "Engine/Public/unreal_objects.h"
#include <map>



namespace SDK
{
	class UnrealMember
	{
	public:
		UnrealMember() = default;

	public:
		uobject Owner = nullptr;

		std::string Type = "";
		std::string Name = "";
		uint32_t Offset = 0;
		uint32_t Size = 0;

		bool is_struct = false;
	};
	class UnrealClass
	{
	public:
		UnrealClass() = default;
		
	UnrealClass(uobject BaseObject) : Owner(BaseObject) {}

	public:
		uobject Owner;

		std::string FullName = "";
		std::string CppName = "";
		std::string StructName = "";
		std::uint32_t InheritedSize = 0;
		std::uint32_t Size = 0;


		std::map<std::string, uint32_t> Offsets;
	};

	class UnrealPackage
	{
	public:
		UnrealPackage() = default;
		UnrealPackage(uobject* BaseObject, std::vector<uobject*> BaseObjects) : Owner(BaseObject), Objects(BaseObjects){}

	public:
		uobject* Owner;

		std::vector<uobject*> Objects;

		std::unordered_map<std::string, UnrealClass*> Classes;
	};
}