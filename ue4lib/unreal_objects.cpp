#include <format>

#include "unreal_objects.h"
#include "Offsets.h"
#include "object_array.h"

#include "settings.h"

std::unordered_map<std::string, ufunction> cached_functions;
void* ffieldclass::get_address()
{
	return Class;
}

ffieldclass::operator bool() const
{
	return Class != nullptr;
}

EFieldClassID ffieldclass::get_id() const
{
	return *reinterpret_cast<EFieldClassID*>(Class + offsets::ffieldclass::id);
}

EClassCastFlags ffieldclass::get_cast_flags() const
{
	return *reinterpret_cast<EClassCastFlags*>(Class + offsets::ffieldclass::cast_flags);
}

EClassFlags ffieldclass::get_class_flags() const
{
	return *reinterpret_cast<EClassFlags*>(Class + offsets::ffieldclass::cast_flags);
}

ffieldclass ffieldclass::get_Super() const
{
	return ffieldclass(*reinterpret_cast<void**>(Class + offsets::ffieldclass::super_class));
}

fname ffieldclass::get_fname() const
{
	return fname(Class + offsets::ffieldclass::name); //Not the real FName, but a wrapper which holds the address of a FName
}

bool ffieldclass::is_type(EClassCastFlags Flags) const
{
	return (Flags != EClassCastFlags::None ? (get_cast_flags() & Flags) : true);
}

std::string ffieldclass::get_name() const
{
	return Class ? get_fname().to_string() : "None";
}

std::string ffieldclass::get_valid_name() const
{
	return Class ? get_fname().to_valid_string() : "None";
}

std::string ffieldclass::get_cpp_name() const
{
	// This is evile dark magic code which shouldn't exist
	return "F" + get_valid_name();
}

void* ffield::get_address()
{
	return Field;
}

EObjectFlags ffield::get_flags() const
{
	return *reinterpret_cast<EObjectFlags*>(Field + offsets::ffield::flags);
}

class uobject ffield::get_owner_as_uobject() const
{
	if (is_owner_uobject())
	{
		if (Settings::Internal::bUseMaskForFieldOwner)
			return (void*)(*reinterpret_cast<uintptr_t*>(Field + offsets::ffield::owner) & ~0x1ull);

		return *reinterpret_cast<void**>(Field + offsets::ffield::owner);
	}

	return nullptr;
}

class ffield ffield::get_owner_as_ffield() const
{
	if (!is_owner_uobject())
		return *reinterpret_cast<void**>(Field + offsets::ffield::owner);

	return nullptr;
}

class uobject ffield::get_owner_uobject() const
{
	ffield Field = *this;

	while (!Field.is_owner_uobject() && Field.get_owner_as_ffield())
	{
		Field = Field.get_owner_as_ffield();
	}

	return Field.get_owner_as_uobject();
}

uobject ffield::get_outer_private() const
{
	return uobject(*(void**)(Field + offsets::uobjecto::outer));
}

ffieldclass ffield::get_class() const
{
	return ffieldclass(*reinterpret_cast<void**>(Field + offsets::ffield::klass));
}

fname ffield::get_fname() const
{
	return fname(Field + offsets::ffield::name); //Not the real FName, but a wrapper which holds the address of a FName
}

ffield ffield::get_next() const
{
	return ffield(*reinterpret_cast<void**>(Field + offsets::ffield::next));
}

template<typename UEType>
UEType ffield::cast() const
{
	return UEType(Field);
}

bool ffield::is_owner_uobject() const
{
	if (Settings::Internal::bUseMaskForFieldOwner)
	{
		return *reinterpret_cast<uintptr_t*>(Field + offsets::ffield::owner) & 0x1;
	}

	return *reinterpret_cast<bool*>(Field + offsets::ffield::owner + 0x8);
}

bool ffield::isa(EClassCastFlags Flags) const
{
	return (Flags != EClassCastFlags::None ? get_class().is_type(Flags) : true);
}

std::string ffield::get_name() const
{
	return Field ? get_fname().to_string() : "None";
}

std::string ffield::get_valid_name() const
{
	return Field ? get_fname().to_valid_string() : "None";
}

std::string ffield::get_cpp_name() const
{
	static uclass ActorClass = object_array::find_class_fast("Actor");
	static uclass InterfaceClass = object_array::find_class_fast("Interface");

	std::string Temp = get_valid_name();

	if (isa(EClassCastFlags::Class))
	{
		if (cast<uclass>().has_type(ActorClass))
		{
			return 'A' + Temp;
		}
		else if (cast<uclass>().has_type(InterfaceClass))
		{
			return 'I' + Temp;
		}

		return 'U' + Temp;
	}

	return 'F' + Temp;
}

ffield::operator bool() const
{
	return Field != nullptr && reinterpret_cast<void*>(Field + offsets::ffield::klass) != nullptr;
}

bool ffield::operator==(const ffield& Other) const
{
	return Field == Other.Field;
}

bool ffield::operator!=(const ffield& Other) const
{
	return Field != Other.Field;
}

void(*uobject::pe)(void*, void*, void*) = nullptr;

void* uobject::get_address()
{
	return Object;
}

void** uobject::get_vft() const
{
	return *reinterpret_cast<void***>(Object);
}

EObjectFlags uobject::get_flags() const
{
	return *reinterpret_cast<EObjectFlags*>(Object + offsets::uobjecto::flags);
}

int32 uobject::get_index() const
{
	return *reinterpret_cast<int32*>(Object + offsets::uobjecto::index);
}

uclass uobject::get_class() const
{
	return uclass(*reinterpret_cast<void**>(Object + offsets::uobjecto::klass));
}

fname uobject::get_fname() const
{
	return fname(Object + offsets::uobjecto::name); //Not the real FName, but a wrapper which holds the address of a FName
}

uobject uobject::get_outer() const
{
	return uobject(*reinterpret_cast<void**>(Object + offsets::uobjecto::outer));
}
uobject uobject::get_package()
{
	return get_outer_most();
}


int32 uobject::get_package_index() const
{
	return get_outer_most().get_index();
}

bool uobject::has_any_flags(EObjectFlags Flags) const
{
	return get_flags() & Flags;
}

bool uobject::isa(EClassCastFlags TypeFlags) const
{
	return (TypeFlags != EClassCastFlags::None ? get_class().is_type(TypeFlags) : true);
}

bool uobject::isa(uclass Class) const
{
	if (!Class)
		return false;

	for (uclass Clss = get_class(); Clss; Clss = Clss.get_super().cast<uclass>())
	{
		if (Clss == Class)
			return true;
	}

	return false;
}

uobject uobject::get_outer_most() const
{
	uobject Outermost = *this;

	for (uobject Outer = *this; Outer; Outer = Outer.get_outer())
	{
		Outermost = Outer;
	}

	return Outermost;
}

std::string uobject::stringifgy_object_flags() const
{
	return *this ? StringifyObjectFlags(get_flags()) : "NoFlags";
}

std::string uobject::get_name() const
{
	return Object ? get_fname().to_string() : "None";
}

std::string uobject::get_valid_name() const
{
	return Object ? get_fname().to_valid_string() : "None";
}

std::string uobject::get_cpp_name() const
{
	static uclass ActorClass = nullptr;
	static uclass InterfaceClass = nullptr;

	if (ActorClass == nullptr)
		ActorClass = object_array::find_class_fast("Actor");

	if (InterfaceClass == nullptr)
		InterfaceClass = object_array::find_class_fast("Interface");

	std::string Temp = get_valid_name();

	if (isa(EClassCastFlags::Class))
	{
		if (cast<uclass>().has_type(ActorClass))
		{
			return 'A' + Temp;
		}
		else if (cast<uclass>().has_type(InterfaceClass))
		{
			return 'I' + Temp;
		}

		return 'U' + Temp;
	}

	return 'F' + Temp;
}

std::string uobject::get_full_name(int32& OutNameLength)
{
	if (*this)
	{
		std::string Temp;

		for (uobject Outer = get_outer(); Outer; Outer = Outer.get_outer())
		{
			Temp = Outer.get_name() + "." + Temp;
		}

		std::string Name = get_name();
		OutNameLength = Name.size() + 1;

		Name = get_class().get_name() + " " + Temp + Name;

		return Name;
	}

	return "None";
}

std::string uobject::get_full_name() const
{
	if (*this)
	{
		std::string Temp;

		for (uobject Outer = get_outer(); Outer; Outer = Outer.get_outer())
		{
			Temp = Outer.get_name() + "." + Temp;
		}

		std::string Name = get_class().get_name();
		Name += " ";
		Name += Temp;
		Name += get_name();

		return Name;
	}

	return "None";
}

uobject::operator bool() const
{
	// if an object is 0x10000F000 it passes the nullptr check
	return Object != nullptr && reinterpret_cast<void*>(Object + offsets::uobjecto::klass) != nullptr;
}

uobject::operator uint8* ()
{
	return Object;
}

bool uobject::operator==(const uobject& Other) const
{
	return Object == Other.Object;
}

bool uobject::operator!=(const uobject& Other) const
{
	return Object != Other.Object;
}

void uobject::process_event(ufunction Func, void* Params)
{
	void** VFT = *reinterpret_cast<void***>(get_address());

	void(*Prd)(void*, void*, void*) = decltype(Prd)(VFT[offsets::process_event::pe_index]);

	Prd(Object, Func.get_address(), Params);
}

ufield ufield::get_next() const
{
	return ufield(*reinterpret_cast<void**>(Object + offsets::ufieldo::next));
}

bool ufield::is_next_valid() const
{
	return (bool)get_next();
}

std::vector<std::pair<fname, int64>> uenum::get_name_value_pairs() const
{
	struct alignas(0x4) Name08Byte { uint8 Pad[0x08]; };
	struct alignas(0x4) Name16Byte { uint8 Pad[0x10]; };

	std::vector<std::pair<fname, int64>> Ret;

	if (!Settings::Internal::bIsEnumNameOnly)
	{
		if (Settings::Internal::bUseCasePreservingName)
		{
			auto& Names = *reinterpret_cast<tarray<tpair<Name16Byte, int64>>*>(Object + offsets::uenumo::names);

			for (int i = 0; i < Names.Num(); i++)
			{
				Ret.push_back({ fname(&Names[i].First), Names[i].Second });
			}
		}
		else
		{
			auto& Names = *reinterpret_cast<tarray<tpair<Name08Byte, int64>>*>(Object + offsets::uenumo::names);

			for (int i = 0; i < Names.Num(); i++)
			{
				Ret.push_back({ fname(&Names[i].First), Names[i].Second });
			}
		}
	}
	else
	{
		auto& NameOnly = *reinterpret_cast<tarray<fname>*>(Object + offsets::uenumo::names);

		if (Settings::Internal::bUseCasePreservingName)
		{
			auto& Names = *reinterpret_cast<tarray<Name16Byte>*>(Object + offsets::uenumo::names);

			for (int i = 0; i < Names.Num(); i++)
			{
				Ret.push_back({ fname(&Names[i]), i });
			}
		}
		else
		{
			auto& Names = *reinterpret_cast<tarray<Name08Byte>*>(Object + offsets::uenumo::names);

			for (int i = 0; i < Names.Num(); i++)
			{
				Ret.push_back({ fname(&Names[i]), i });
			}
		}
	}

	return Ret;
}

std::string uenum::get_single_name(int32 Index) const
{
	return get_name_value_pairs()[Index].first.to_string();
}

std::string uenum::get_enum_prefixed_name() const
{
	std::string Temp = get_valid_name();

	return Temp[0] == 'E' ? Temp : 'E' + Temp;
}

std::string uenum::get_enum_type_as_str() const
{
	return "enum class " + get_enum_prefixed_name();
}

ustruct ustruct::get_super() const
{
	return ustruct(*reinterpret_cast<void**>(Object + offsets::ustructo::super_struct));
}

ufield ustruct::get_child() const
{
	return ufield(*reinterpret_cast<void**>(Object + offsets::ustructo::children));
}

ffield ustruct::get_child_properties() const
{
	return ffield(*reinterpret_cast<void**>(Object + offsets::ustructo::child_properties));
}

int32 ustruct::get_min_alingment() const
{
	return *reinterpret_cast<int32*>(Object + offsets::ustructo::min_alignment);
}

int32 ustruct::get_struct_size() const
{
	return *reinterpret_cast<int32*>(Object + offsets::ustructo::size);
}

std::vector<uproperty> ustruct::get_properties() const
{
	std::vector<uproperty> Properties;

	if (Settings::Internal::bUseFProperty)
	{
		for (ffield Field = get_child_properties(); Field; Field = Field.get_next())
		{
			if (Field.isa(EClassCastFlags::Property))
				Properties.push_back(Field.cast<uproperty>());
		}

		return Properties;
	}

	for (ufield Field = get_child(); Field; Field = Field.get_next())
	{
		if (Field.isa(EClassCastFlags::Property))
			Properties.push_back(Field.cast<uproperty>());
	}

	return Properties;
}

std::vector<ufunction> ustruct::get_functions() const
{
	std::vector<ufunction> Functions;

	for (ufield Field = get_child(); Field; Field = Field.get_next())
	{
		if (Field.isa(EClassCastFlags::Function))
			Functions.push_back(Field.cast<ufunction>());
	}

	return Functions;
}



uproperty ustruct::find_member(const std::string& MemberName, EClassCastFlags TypeFlags) const
{
	if (!Object)
		return nullptr;

	if (Settings::Internal::bUseFProperty)
	{
		for (ffield Field = get_child_properties(); Field; Field = Field.get_next())
		{
			if (Field.isa(TypeFlags) && Field.get_name() == MemberName)
			{
				return Field.cast<uproperty>();
			}
		}
	}

	for (ufield Field = get_child(); Field; Field = Field.get_next())
	{
		if (Field.isa(TypeFlags) && Field.get_name() == MemberName)
		{
			return Field.cast<uproperty>();
		}
	}

	return nullptr;
}

bool ustruct::has_members() const
{
	if (!Object)
		return false;

	if (Settings::Internal::bUseFProperty)
	{
		for (ffield Field = get_child_properties(); Field; Field = Field.get_next())
		{
			if (Field.isa(EClassCastFlags::Property))
				return true;
		}
	}
	else
	{
		for (ufield F = get_child(); F; F = F.get_next())
		{
			if (F.isa(EClassCastFlags::Property))
				return true;
		}
	}

	return false;
}

EClassCastFlags uclass::get_cast_flags() const
{
	return *reinterpret_cast<EClassCastFlags*>(Object + offsets::uclasso::cast_flags);
}

std::string uclass::stringify_cast_flags() const
{
	return StringifyClassCastFlags(get_cast_flags());
}

bool uclass::is_type(EClassCastFlags TypeFlag) const
{
	return (TypeFlag != EClassCastFlags::None ? (get_cast_flags() & TypeFlag) : true);
}

bool uclass::has_type(uclass TypeClass) const
{
	if (TypeClass == nullptr)
		return false;

	for (ustruct S = *this; S; S = S.get_super())
	{
		if (S == TypeClass)
			return true;
	}

	return false;
}

uobject uclass::get_default_object() const
{
	return uobject(*reinterpret_cast<void**>(Object + offsets::uclasso::class_default_object));
}

ufunction uclass::get_function(const std::string& ClassName, const std::string& FuncName) const
{
	for (ustruct Struct = *this; Struct; Struct = Struct.get_super())
	{
		if (Struct.get_name() != ClassName)
			continue;

		for (ufield Field = Struct.get_child(); Field; Field = Field.get_next())
		{
			if (Field.isa(EClassCastFlags::Function) && Field.get_name() == FuncName)
			{
				return Field.cast<ufunction>();
			}
		}

	}

	return nullptr;
}

EFunctionFlags ufunction::get_function_flags() const
{
	return *reinterpret_cast<EFunctionFlags*>(Object + offsets::ufunctiono::function_flags);
}

bool ufunction::has_flags(EFunctionFlags FuncFlags) const
{
	return get_function_flags() & FuncFlags;
}

void* ufunction::get_exec_function() const
{
	return *reinterpret_cast<void**>(Object + offsets::ufunctiono::exec_function);
}

uproperty ufunction::get_return_property() const
{
	for (auto Prop : get_properties())
	{
		if (Prop.has_property_flags(EPropertyFlags::ReturnParm))
			return Prop;
	}

	return nullptr;
}


std::string ufunction::stringify_Flags(const char* Seperator)  const
{
	return StringifyFunctionFlags(get_function_flags(), Seperator);
}

std::string ufunction::get_param_struct_name() const
{
	return get_outer().get_cpp_name() + "_" + get_valid_name() + "_Params";
}

void* uproperty::get_address()
{
	return Base;
}

std::pair<uclass, ffieldclass> uproperty::get_class() const
{
	if (Settings::Internal::bUseFProperty)
	{
		return { uclass(0), ffield(Base).get_class() };
	}

	return { uobject(Base).get_class(), ffieldclass(0) };
}

EClassCastFlags uproperty::get_cast_flags() const
{
	auto [Class, FieldClass] = get_class();

	return Class ? Class.get_cast_flags() : FieldClass.get_cast_flags();
}

uproperty::operator bool() const
{
	return Base != nullptr && ((Base + offsets::uobjecto::klass) != nullptr || (Base + offsets::ffield::klass) != nullptr);
}



bool uproperty::isa(EClassCastFlags TypeFlags) const
{
	if (get_class().first)
		return get_class().first.is_type(TypeFlags);

	return get_class().second.is_type(TypeFlags);
}

fname uproperty::get_fname() const
{
	if (Settings::Internal::bUseFProperty)
	{
		return fname(Base + offsets::ffield::name); //Not the real FName, but a wrapper which holds the address of a FName
	}

	return fname(Base + offsets::uobjecto::name); //Not the real FName, but a wrapper which holds the address of a FName
}

int32 uproperty::get_array_dim() const
{
	return *reinterpret_cast<int32*>(Base + offsets::fproperty::array_dim);
}

int32 uproperty::get_size() const
{
	return *reinterpret_cast<int32*>(Base + offsets::fproperty::elements_size);
}

int32 uproperty::get_offset() const
{
	return *reinterpret_cast<int32*>(Base + offsets::fproperty::offset_internal);
}

EPropertyFlags uproperty::get_property_flags() const
{
	return *reinterpret_cast<EPropertyFlags*>(Base + offsets::fproperty::property_flags);
}

bool uproperty::has_property_flags(EPropertyFlags PropertyFlag) const
{
	return get_property_flags() & PropertyFlag;
}

bool uproperty::is_type(EClassCastFlags PossibleTypes) const
{
	return (static_cast<uint64>(get_cast_flags()) & static_cast<uint64>(PossibleTypes)) != 0;
}

std::string uproperty::get_name() const
{
	return Base ? get_fname().to_string() : "None";
}

std::string uproperty::get_valid_name() const
{
	return Base ? get_fname().to_valid_string() : "None";
}

int32 uproperty::get_alignment() const
{
	EClassCastFlags TypeFlags = (get_class().first ? get_class().first.get_cast_flags() : get_class().second.get_cast_flags());

	if (TypeFlags & EClassCastFlags::ByteProperty)
	{
		return alignof(uint8); // 0x1
	}
	else if (TypeFlags & EClassCastFlags::UInt16Property)
	{
		return alignof(uint16); // 0x2
	}
	else if (TypeFlags & EClassCastFlags::UInt32Property)
	{
		return alignof(uint32); // 0x4
	}
	else if (TypeFlags & EClassCastFlags::UInt64Property)
	{
		return alignof(uint64); // 0x8
	}
	else if (TypeFlags & EClassCastFlags::Int8Property)
	{
		return alignof(int8); // 0x1
	}
	else if (TypeFlags & EClassCastFlags::Int16Property)
	{
		return alignof(int16); // 0x2
	}
	else if (TypeFlags & EClassCastFlags::IntProperty)
	{
		return alignof(int32); // 0x4
	}
	else if (TypeFlags & EClassCastFlags::Int64Property)
	{
		return alignof(int64); // 0x8
	}
	else if (TypeFlags & EClassCastFlags::FloatProperty)
	{
		return alignof(float); // 0x4
	}
	else if (TypeFlags & EClassCastFlags::DoubleProperty)
	{
		return alignof(double); // 0x8
	}
	else if (TypeFlags & EClassCastFlags::ClassProperty)
	{
		return alignof(void*); // 0x8
	}
	else if (TypeFlags & EClassCastFlags::NameProperty)
	{
		return 0x4; // FName is a bunch of int32s
	}
	else if (TypeFlags & EClassCastFlags::StrProperty)
	{
		return alignof(fstring); // 0x8
	}
	else if (TypeFlags & EClassCastFlags::TextProperty)
	{
		return 0x8; // alignof member FString
	}
	else if (TypeFlags & EClassCastFlags::BoolProperty)
	{
		return alignof(bool); // 0x1
	}
	else if (TypeFlags & EClassCastFlags::StructProperty)
	{
		return cast<ustructproperty>().get_underlaying_struct().get_min_alingment();
	}
	else if (TypeFlags & EClassCastFlags::ArrayProperty)
	{
		return alignof(tarray<int>); // 0x8
	}
	else if (TypeFlags & EClassCastFlags::DelegateProperty)
	{
		return alignof(int32); // 0x4
	}
	else if (TypeFlags & EClassCastFlags::WeakObjectProperty)
	{
		return 0x4; // TWeakObjectPtr is a bunch of int32s
	}
	else if (TypeFlags & EClassCastFlags::LazyObjectProperty)
	{
		return 0x4; // TLazyObjectPtr is a bunch of int32s
	}
	else if (TypeFlags & EClassCastFlags::SoftClassProperty)
	{
		return 0x8; // alignof member FString
	}
	else if (TypeFlags & EClassCastFlags::SoftObjectProperty)
	{
		return 0x8; // alignof member FString
	}
	else if (TypeFlags & EClassCastFlags::ObjectProperty)
	{
		return alignof(void*); // 0x8
	}
	else if (TypeFlags & EClassCastFlags::MapProperty)
	{
		return alignof(tarray<int>); // 0x8, TMap contains a TArray
	}
	else if (TypeFlags & EClassCastFlags::SetProperty)
	{
		return alignof(tarray<int>); // 0x8, TSet contains a TArray
	}
	else if (TypeFlags & EClassCastFlags::EnumProperty)
	{
		uproperty P = cast<uenumproperty>().get_underlaying_property();

		return P ? P.get_alignment() : 0x1;
	}
	else if (TypeFlags & EClassCastFlags::InterfaceProperty)
	{
		return alignof(void*); // 0x8
	}
	else if (TypeFlags & EClassCastFlags::FieldPathProperty)
	{
		return 0x8; // alignof member TArray<FName> and ptr;
	}
	else if (TypeFlags & EClassCastFlags::MulticastSparseDelegateProperty)
	{
		return 0x1; // size in PropertyFixup (alignment isn't greater than size)
	}
	else if (TypeFlags & EClassCastFlags::MulticastInlineDelegateProperty)
	{
		return 0x8;  // alignof member TArray<FName>
	}
	else if (TypeFlags & EClassCastFlags::OptionalProperty)
	{
		uproperty Valuproperty = cast<UEOptionalProperty>().get_value_property();

		/* If this check is true it means, that there is no bool in this TOptional to check if the value is set */
		if (Valuproperty.get_size() == get_size()) [[unlikely]]
			return Valuproperty.get_alignment();

			return  get_size() - Valuproperty.get_size();
	}

	if (Settings::Internal::bUseFProperty)
	{
		static std::unordered_map<void*, int32> UnknownProperties;

		static auto TryFindPropertyRefInOptionalToGetAlignment = [](std::unordered_map<void*, int32>& OutProperties, void* PropertyClass) -> int32
			{
				/* Search for a TOptionalProperty that contains an instance of this property */
				for (uobject Obj : object_array())
				{
					if (!Obj.isa(EClassCastFlags::Struct))
						continue;

					for (uproperty Prop : Obj.cast<ustruct>().get_properties())
					{
						if (!Prop.isa(EClassCastFlags::OptionalProperty))
							continue;

						UEOptionalProperty Optional = Prop.cast<UEOptionalProperty>();

						/* Safe to use first member, as we're guaranteed to use FProperty */
						if (Optional.get_value_property().get_class().second.get_address() == PropertyClass)
							return OutProperties.insert({ PropertyClass, Optional.get_alignment() }).first->second;
					}
				}

				return OutProperties.insert({ PropertyClass, 0x1 }).first->second;
			};

		auto It = UnknownProperties.find(get_class().second.get_address());

		/* Safe to use first member, as we're guaranteed to use FProperty */
		if (It == UnknownProperties.end())
			return TryFindPropertyRefInOptionalToGetAlignment(UnknownProperties, get_class().second.get_address());

		return It->second;
	}

	return 0x1;
}

std::string uproperty::get_cpp_type() const
{
	EClassCastFlags TypeFlags = (get_class().first ? get_class().first.get_cast_flags() : get_class().second.get_cast_flags());
	if (TypeFlags & EClassCastFlags::ByteProperty)
	{
		return cast<ubyteproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::UInt16Property)
	{
		return "uint16_t";
	}
	else if (TypeFlags & EClassCastFlags::UInt32Property)
	{
		return "uint32_t";
	}
	else if (TypeFlags & EClassCastFlags::UInt64Property)
	{
		return "uint64_t";
	}
	else if (TypeFlags & EClassCastFlags::Int8Property)
	{
		return "int8_t";
	}
	else if (TypeFlags & EClassCastFlags::Int16Property)
	{
		return "int16_t";
	}
	else if (TypeFlags & EClassCastFlags::IntProperty)
	{
		return "int32_t";
	}
	else if (TypeFlags & EClassCastFlags::Int64Property)
	{
		return "int64_t";
	}
	else if (TypeFlags & EClassCastFlags::FloatProperty)
	{
		return "float";
	}
	else if (TypeFlags & EClassCastFlags::DoubleProperty)
	{
		return "double";
	}
	else if (TypeFlags & EClassCastFlags::ClassProperty)
	{
		return cast<uclassproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::NameProperty)
	{
		return "class FName";
	}
	else if (TypeFlags & EClassCastFlags::StrProperty)
	{
		return "class FString";
	}
	else if (TypeFlags & EClassCastFlags::TextProperty)
	{
		return "class FText";
	}
	else if (TypeFlags & EClassCastFlags::BoolProperty)
	{	
		return cast<uboolproperty>().GetCppType();
	}
	else if (TypeFlags & EClassCastFlags::StructProperty)
	{
		return cast<ustructproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::ArrayProperty)
	{
		return cast<uarrayproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::WeakObjectProperty)
	{
		return cast<uweakobjectproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::LazyObjectProperty)
	{
		return cast<ulazyobjectproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::SoftClassProperty)
	{
		return cast<UESoftClassProperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::SoftObjectProperty)
	{
		return cast<usoftobjectproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::ObjectProperty)
	{
		return cast<uobjectproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::MapProperty)
	{
		return cast<umapproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::SetProperty)
	{
		return cast<usetproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::EnumProperty)
	{
		return cast<uenumproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::InterfaceProperty)
	{
		return cast<uinterfaceproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::FieldPathProperty)
	{
		return cast<UEFieldPathProperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::DelegateProperty)
	{
		return cast<udelegateproperty>().get_cpp_type();
	}
	else if (TypeFlags & EClassCastFlags::OptionalProperty)
	{
		return cast<UEOptionalProperty>().get_cpp_type();
	}
	else
	{
		return (get_class().first ? get_class().first.get_cpp_name() : get_class().second.get_cpp_name()) + "_";;
	}

	return "";
}

std::string uproperty::stringify_flags() const
{
	return StringifyPropertyFlags(get_property_flags());
}

uint32_t uproperty::property_size() const
{
	return *reinterpret_cast<uint32*>(Base + offsets::properties::property_size);
}

uobject uproperty::static_class()
{
	static uobject obj = 0;
	if (!obj) obj = object_array::find_objects("Class CoreUObject.Property");
	return obj;
}

uenum ubyteproperty::get_enum() const
{
	return uenum(*reinterpret_cast<void**>(Base + offsets::byte_property::senum));
}

std::string ubyteproperty::get_cpp_type() const
{
	if (uenum Enum = get_enum())
	{
		return Enum.get_enum_type_as_str();
	}

	return "uint8_t";
}

uint8 uboolproperty::GetFieldMask() const
{
	return reinterpret_cast<offsets::bool_property::ubool_propety_base*>(Base + offsets::bool_property::base)->field_mask;
}

uint8 uboolproperty::GetBitIndex() const
{
	uint8 FieldMask = GetFieldMask();

	if (FieldMask != 0xFF)
	{
		if (FieldMask == 0x01) { return 0; }
		if (FieldMask == 0x02) { return 1; }
		if (FieldMask == 0x04) { return 2; }
		if (FieldMask == 0x08) { return 3; }
		if (FieldMask == 0x10) { return 4; }
		if (FieldMask == 0x20) { return 5; }
		if (FieldMask == 0x40) { return 6; }
		if (FieldMask == 0x80) { return 7; }
	}

	return 0xFF;
}

bool uboolproperty::IsNativeBool() const
{
	return reinterpret_cast<offsets::bool_property::ubool_propety_base*>(Base + offsets::bool_property::base)->field_mask == 0xFF;
}

std::string uboolproperty::GetCppType() const
{
	return IsNativeBool() ? "bool" : "uint8";
}

uclass uobjectproperty::get_property_class() const
{
	return uclass(*reinterpret_cast<void**>(Base + offsets::object_property::property_class));
}

std::string uobjectproperty::get_cpp_type() const
{
	return std::format("class {}*", get_property_class() ? get_property_class().get_cpp_name() : "UObject");
}

uclass uclassproperty::get_meta_class() const
{
	return uclass(*reinterpret_cast<void**>(Base + offsets::class_property::meta_class));
}

std::string uclassproperty::get_cpp_type() const
{
	return has_property_flags(EPropertyFlags::UObjectWrapper) ? std::format("TSubclassOf<class {}>", get_meta_class().get_cpp_name()) : "class UClass*";
}

std::string uweakobjectproperty::get_cpp_type() const
{
	return std::format("TWeakObjectPtr<class {}>", get_property_class() ? get_property_class().get_cpp_name() : "UObject");
}

std::string ulazyobjectproperty::get_cpp_type() const
{
	return std::format("TLazyObjectPtr<class {}>", get_property_class() ? get_property_class().get_cpp_name() : "UObject");
}
std::string usoftobjectproperty::get_cpp_type() const
{
	return std::format("TSoftObjectPtr<class {}>", get_property_class() ? get_property_class().get_cpp_name() : "UObject");
}
std::string UESoftClassProperty::get_cpp_type() const
{
	return std::format("TSoftClassPtr<class {}>", get_meta_class() ? get_meta_class().get_cpp_name() : get_property_class().get_cpp_name());
}
std::string uinterfaceproperty::get_cpp_type() const
{
	return std::format("TScriptInterface<class {}>", get_property_class().get_cpp_name());
}
ustruct ustructproperty::get_underlaying_struct() const
{
	return ustruct(*reinterpret_cast<void**>(Base + offsets::struct_property::sstruct));
}
std::string ustructproperty::get_cpp_type() const
{
	return std::format("struct {}", get_underlaying_struct().get_cpp_name());
}
uproperty uarrayproperty::get_inner_property() const
{
	return uproperty(*reinterpret_cast<void**>(Base + offsets::array_property::inner));
}
std::string uarrayproperty::get_cpp_type() const
{
	return std::format("TArray<{}>", get_inner_property().get_cpp_type());
}
ufunction udelegateproperty::get_signature_function() const
{
	return ufunction(*reinterpret_cast<void**>(Base + offsets::delegate_property::signature_function));
}
std::string udelegateproperty::get_cpp_type() const
{
	return "TDeleage<get_cpp_typeIsNotImplementedForDelegates>";
}
uproperty umapproperty::get_key_property() const
{
	return uproperty(reinterpret_cast<offsets::map_property::umap_property_base*>(Base + offsets::map_property::base)->key_property);
}
uproperty umapproperty::get_value_property() const
{
	return uproperty(reinterpret_cast<offsets::map_property::umap_property_base*>(Base + offsets::map_property::base)->value_property);
}
std::string umapproperty::get_cpp_type() const
{
	return std::format("TMap<{}, {}>", get_key_property().get_cpp_type(), get_value_property().get_cpp_type());
}
uproperty usetproperty::get_element_property() const
{
	return uproperty(*reinterpret_cast<void**>(Base + offsets::set_property::element_prop));
}
std::string usetproperty::get_cpp_type() const
{
	return std::format("TSet<{}>", get_element_property().get_cpp_type());
}
uproperty uenumproperty::get_underlaying_property() const
{
	return uproperty(reinterpret_cast<offsets::enum_property::uenum_property_base*>(Base + offsets::enum_property::base)->underlaying_property);
}
uenum uenumproperty::get_enum() const
{
	return uenum(reinterpret_cast<offsets::enum_property::uenum_property_base*>(Base + offsets::enum_property::base)->senum);
}
std::string uenumproperty::get_cpp_type() const
{
	if (get_enum())
		return get_enum().get_enum_type_as_str();
	return get_underlaying_property().get_cpp_type();
}
ffieldclass UEFieldPathProperty::get_field_class() const
{
	return ffieldclass(*reinterpret_cast<void**>(Base + offsets::field_path_property::field_class));
}
std::string UEFieldPathProperty::get_cpp_type() const
{
	return std::format("TFieldPath<struct {}>", get_field_class().get_cpp_name());
}
uproperty UEOptionalProperty::get_value_property() const
{
	return uproperty(*reinterpret_cast<void**>(Base + offsets::optional_property::value_property));
}
std::string UEOptionalProperty::get_cpp_type() const
{
	return std::format("TOptional<{}>", get_value_property().get_cpp_type());
}
uobject uenum::static_class()
{
	static uobject obj = 0;
	if (!obj) obj = object_array::find_objects("Class CoreUObject.Enum");
	return obj;
}
uobject uclass::static_class()
{
	static uobject obj = 0;
	if (!obj) obj = object_array::find_objects("Class CoreUObject.Class");
	return obj;
}
uobject ustruct::static_class()
{
	static uobject obj = 0;
	if (!obj) obj = object_array::find_objects("Class CoreUObject.Struct");
	return obj;
}
uobject ufunction::static_class()
{
	static uobject obj = 0;
	if (!obj) obj = object_array::find_objects("Class CoreUObject.Function");
	return obj;
}

ufunction ufunction::find_function_objects(std::string name)
{
	if (cached_functions.find(name) == cached_functions.end())
		cached_functions[name] = object_array::find_objects(name).cast<ufunction>();
	return cached_functions.at(name);
}
