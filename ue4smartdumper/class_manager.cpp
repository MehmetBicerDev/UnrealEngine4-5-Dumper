#include "class_manager.h"
#include "unreal_struct.h"


namespace dumper {
	namespace class_manager {
		void dump_property(uproperty object, unreal_struct* s)
		{
			unreal_member* member = new unreal_member();
			if (!object.get_address()) return;
			EClassCastFlags type_flags = (object.get_class().first ? object.get_class().first.get_cast_flags() : object.get_class().second.get_cast_flags());

			auto dim = object.get_array_dim();
			member->Name = object.get_fname().to_string();

			member->Type = object.get_cpp_type();

			member->Offset = object.get_offset();
			member->Size = object.get_size() * dim;

			if (!member->Size) return;
			if (member->Offset > s->Offset) {
				s->OffsetPadding(member->Offset - s->Offset, member->Offset);
			}

			if (type_flags == EClassCastFlags::BoolProperty && (*(std::uint32_t*)(member->Type.data()) != 'loob'))
			{
				s->FillBits(object.get_mask(), member);
			}
			else {
				if (dim > 1) {
					std::stringstream stream;
					stream << std::hex << dim;
					std::string result(stream.str());
					member->Name += "[" + result + "]";// fmt::format("[{0:x}]", dim);

					//	std::cout << s->FullName << " " << Member->Name << std::endl;
				}

				s->Offset += member->Size;
			}
			s->Members.push_back(member);
		}

		void dump_function(ufunction object, unreal_struct* s)
		{
			unreal_function* func = new unreal_function();

			func->Owner = object;
			func->FullName = object.get_full_name();

			auto prop = object.get_child_properties();

			while (prop.get_address()) {

				func->GenerateParam(prop.cast<uproperty>());
				prop = prop.get_next();
			}

			prop = object.get_child().cast<ffield>();

			while (prop.get_address()) {

				func->GenerateParam(prop.cast<uproperty>());

				prop = prop.get_next();
			}



			if (func->params.size()) {
				func->params.erase(func->params.size() - 2);
			}
			if (!func->CppName.size()) {
				func->FuncName = object.get_name();
				func->CppName = "void " + func->FuncName;

				func->FuncType = "void";
			}


			s->Functions.push_back(func);
		}
		void process_class(unreal_package* package, uobject& object, bool is_class)
		{
			unreal_struct* s = new unreal_struct();

			auto object_as_struct = object.cast<ustruct>();

			s->Owner = object;
			s->Size = object_as_struct.get_struct_size();;
			s->FullName = object.get_full_name();
			s->CppName = object.get_cpp_name();
			s->StructName = s->CppName;

			std::string klass = object.get_cpp_name();
			auto properties = object_as_struct.get_properties();
			auto size = object_as_struct.get_struct_size();

			if (!size) return;

			
			auto super = object_as_struct.get_super();

			if (super)
			{
				klass += " : public " + super.get_cpp_name() + "\n{\n";
			}


			for (auto prop : object_as_struct.get_properties()) {
				if (!prop.get_address()) break;

				dump_property(prop, s);

				//prop = prop->OuterPrivate->Cast<uproperty>();
			}

			for (auto func : object_as_struct.get_functions())
			{
				dump_function(func, s);
			}
			
			if (s->Size > s->Offset) {


				s->OffsetPadding(s->Size - s->Offset, s->Size);

			}

			if (is_class)
				package->Classes[s->CppName] = s;
			else
				package->Structs[s->CppName] = s;

				

		}
	}
}