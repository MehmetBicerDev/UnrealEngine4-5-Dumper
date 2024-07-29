#include "unreal_struct.h"
#include "class_manager.h"
void dumper::unreal_function::GenerateParam(uproperty sproperty)
{
	auto flags = sproperty.get_property_flags();

	if (flags & EPropertyFlags::ReturnParm) {
		FuncType = sproperty.get_cpp_type();
		FuncName = Owner.get_name();
		CppName = FuncType + " " + FuncName;
	}
	else if (flags & EPropertyFlags::Parm) {
		std::string property_name = sproperty.get_fname().to_string();
		std::string type = sproperty.get_cpp_type();
		if (sproperty.get_array_dim() > 1) {
			params += type + "* " + property_name + ", ";
			paramlist.push_back({ type + "* " , property_name });
		}
		else {
			if (flags & EPropertyFlags::OutParm) {
				params += type + "& " + property_name + ", ";
				paramlist.push_back({ type + "& " , property_name });

			}
			else {

				//	if (type.find("struct ") != std::string::npos) {
				//		params += " UObject* " + property_name + "/* " + type + "*/, ";
				//		paramlist.push_back({ "UObject* ", property_name});

				//	}
				//	else {
				params += type + " " + property_name + ", ";
				paramlist.push_back({ type + " " , property_name });
				//	}
			}
		}
	}
}

void dumper::unreal_struct::FillBits(std::uint8_t mask, unreal_member* Member)
{
	uint8_t zeros = 0, ones = 0;
	while (mask & ~1) {
		mask >>= 1;
		zeros += 1;
	}
	while (mask & 1) {
		mask >>= 1;
		ones += 1;
	}
	if (zeros > BitOffset) {
		BitPadding(zeros - BitOffset);
		BitOffset = zeros;
	}
	Member->Name += " : " + std::to_string(ones);

	BitOffset += ones;

	if (StructName == "UWorld")
		printf("Current Bit Offset [%x] ones [%d] zeros [%d] \n", BitOffset, ones, zeros);
	if (BitOffset >= 8) {

		printf("BIT OFFSET [%s] [%x] \n", StructName.c_str(), Offset);
		Offset += 1;
		BitOffset = 0;
	}
}

void dumper::unreal_struct::BitPadding(std::uint8_t size)
{
	unreal_member* member = new unreal_member();

	++Paddings[Offset];
	member->Owner = Owner;
	member->Type = "char";
	member->Name = "pad_" + std::to_string(Offset) + "_" + std::to_string(Paddings[Offset]) + " : " + std::to_string(size);// fmt::format("pad_{} : {}", Offset, size);
	member->Offset = Offset;
	member->Size = 1;

	Members.push_back(member);
}

void dumper::unreal_struct::OffsetPadding(size_t pad, size_t memoryoffset)
{
	if (BitOffset && BitOffset < 8)
	{
		BitPadding(8 - BitOffset);
		BitOffset = 0;
		Offset += 1;

	}
	unreal_member* member = new unreal_member();

	pad = memoryoffset - Offset;

	if (Offset != memoryoffset) {


		member->Name = "pad_" + std::to_string(Offset) + "[" + std::to_string(pad) + "]";
		member->Type = "char";
		member->Size = pad;
		member->Offset = Offset;

		Offset = memoryoffset;

		Members.push_back(member);

	}
}

void dumper::unreal_struct::AddMember(size_t size, uint64_t offset, std::string type, std::string name)
{
	unreal_member* member = new unreal_member();
	
	member->is_struct = false;
	member->Offset = offset;
	member->Type = type;
	member->Name = name;
	member->Owner = 0;
}

void dumper::unreal_package::process()
{
	for (auto object : Owner->Objects) {
		if (object) {
			if (object->isa(uenum::static_class().cast<uclass>())) {

				//generator::GenerateEnum(object->cast<uenum>(), this);
			}
			else if (object->isa(ustruct::static_class().cast<uclass>()) || object->isa(uclass::static_class().cast<uclass>())) {
				//	std::cout << "Dumping " << Object->GetCppName() << std::endl;
				class_manager::process_class(this, *object, object->isa(uclass::static_class().cast<uclass>()));
			}



		}
	}
}
