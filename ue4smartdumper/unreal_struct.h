#pragma once
#include <unreal_objects.h>
#include <map>




namespace dumper {
	class unreal_enum {
	public:
		inline unreal_enum() {}

		uobject* Owner;

		std::string FullName = "";
		std::string CppName = "";



		std::vector<std::string> Members;

	};
	class unreal_member {
	public:
		inline unreal_member() {}
		uobject Owner = nullptr;

		std::string Type = "";
		std::string Name = "";
		uint32_t Offset = 0;
		uint32_t Size = 0;

		bool is_struct = false;
	};
	class unreal_function {
	public:
		inline unreal_function() {}

		uobject Owner;

		std::string FullName = "";
		std::string CppName = "";

		std::string FuncType = "";
		std::string FuncName = "";
		std::string params = "";

		std::vector<std::pair<std::string, std::string>> paramlist;


	public:
		void GenerateParam(uproperty sproperty);
	};
	class unreal_struct {
	public:
		inline unreal_struct() {}

		uobject Owner;

		std::string FullName = "";
		std::string CppName = "";
		std::string StructName = "";
		std::uint32_t InheritedSize = 0;
		std::uint32_t Size = 0;


		std::vector<unreal_member*> Members;

		std::vector<unreal_function*> Functions;

		std::map<std::uint64_t, std::uint64_t> Paddings;


	public:
		std::uint64_t Offset = 0;
		std::uint8_t BitOffset = 0;

	public:
		void FillBits(std::uint8_t mask, unreal_member* Member);


		void BitPadding(std::uint8_t size);
		void OffsetPadding(size_t pad, size_t memoryoffset);

		void AddMember(size_t size, uint64_t offset, std::string type, std::string name);
	};
	class upackage {
	public:
		upackage(uobject* p_object, std::vector<uobject*> p_objects) : Object(p_object), Objects(p_objects) {

		}
		uobject* Object = nullptr;


		std::vector<uobject*> Objects;
	};
	class unreal_package
	{
	public:
		inline unreal_package(upackage* p) { Owner = p; }

	public:
		upackage* Owner;

		std::unordered_map<std::string, unreal_struct*> Classes;
		std::unordered_map<std::string, unreal_struct*> Structs;

		std::unordered_map<std::string, unreal_enum*> Enums;

	public:
		void process();
	};
}