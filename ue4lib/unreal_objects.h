#pragma once

#include <vector>
#include <unordered_map>
#include "Enums.h"
#include "unreal_types.h"

class uclass;
class ufield;
class uobject;
class uproperty;

class ffieldclass
{
protected:
	uint8* Class;

public:

	ffieldclass() = default;

	ffieldclass(void* NewFieldClass)
		: Class(reinterpret_cast<uint8*>(NewFieldClass))
	{
	}

	ffieldclass(const ffieldclass& OldFieldClass)
		: Class(reinterpret_cast<uint8*>(OldFieldClass.Class))
	{
	}

	void* get_address();

	operator bool() const;

	EFieldClassID get_id() const;

	EClassCastFlags get_cast_flags() const;
	EClassFlags get_class_flags() const;
	ffieldclass get_Super() const;
	fname get_fname() const;

	bool is_type(EClassCastFlags Flags) const;

	std::string get_name() const;
	std::string get_valid_name() const;
	std::string get_cpp_name() const;
};

class ffield
{
protected:
	uint8* Field;

public:

	ffield() = default;

	ffield(void* NewField)
		: Field(reinterpret_cast<uint8*>(NewField))
	{
	}

	ffield(const ffield& OldField)
		: Field(reinterpret_cast<uint8*>(OldField.Field))
	{
	}

	void* get_address();

	EObjectFlags get_flags() const;
	class uobject get_owner_as_uobject() const;
	class ffield get_owner_as_ffield() const;
	class uobject get_owner_uobject() const;
	class uobject get_outer_private() const;
	ffieldclass get_class() const;
	fname get_fname() const;
	ffield get_next() const;

	template<typename utype>
	utype cast() const;

	bool is_owner_uobject() const;
	bool isa(EClassCastFlags Flags) const;

	std::string get_name() const;
	std::string get_valid_name() const;
	std::string get_cpp_name() const;

	explicit operator bool() const;
	bool operator==(const ffield& Other) const;
	bool operator!=(const ffield& Other) const;
};

class uobject
{
private:
	static void(*pe)(void*, void*, void*);

protected:
	uint8* Object;

public:

	uobject()
	{
		Object = reinterpret_cast<uint8_t*>((uintptr_t)this);
	}

	uobject(void* NewObject)
		: Object(reinterpret_cast<uint8*>(NewObject))
	{
	}

	uobject(const uobject&) = default;

	void* get_address();

	void** get_vft() const;
	EObjectFlags get_flags() const;
	int32 get_index() const;
	uclass get_class() const;
	fname get_fname() const;
	uobject get_outer() const;

	uobject get_package();

	int32 get_package_index() const;

	bool has_any_flags(EObjectFlags Flags) const;

	bool isa(EClassCastFlags TypeFlags) const;
	bool isa(uclass Class) const;

	uobject get_outer_most() const;

	std::string stringifgy_object_flags() const;

	std::string get_name() const;
	std::string get_valid_name() const;
	std::string get_cpp_name() const;
	std::string get_full_name(int32& OutNameLength);
	std::string get_full_name() const;

	explicit operator bool() const;
	explicit operator uint8* ();
	bool operator==(const uobject& Other) const;
	bool operator!=(const uobject& Other) const;
	bool operator<(const uobject& Other) const
	{
		return true;
	}
	bool operator>(const uobject& Other) const
	{
		return false;
	}
	bool operator>=(const uobject& Other) const
	{
		return false;
	}
	bool operator<=(const uobject& Other) const
	{
		return false;
	}
	void process_event(class ufunction Func, void* Params);



public:
	template<typename utype>
	inline utype cast()
	{
		return utype(Object);
	}

	template<typename utype>
	inline const utype cast() const
	{
		return utype(Object);
	}
};

class ufield : public uobject
{
	using uobject::uobject;

public:
	ufield get_next() const;
	bool is_next_valid() const;
};

class uenum : public ufield
{
	using ufield::ufield;

public:
	std::vector<std::pair<fname, int64>> get_name_value_pairs() const;
	std::string get_single_name(int32 Index) const;
	std::string get_enum_prefixed_name() const;
	std::string get_enum_type_as_str() const;

	static uobject static_class();
};

class ustruct : public ufield
{
	using ufield::ufield;

public:
	ustruct get_super() const;
	ufield get_child() const;
	ffield get_child_properties() const;
	int32 get_min_alingment() const;
	int32 get_struct_size() const;

	std::vector<uproperty> get_properties() const;
	std::vector<ufunction> get_functions() const;
	class uobject get_outer_private() const;


	uproperty find_member(const std::string& MemberName, EClassCastFlags TypeFlags = EClassCastFlags::None) const;

	bool has_members() const;


	static uobject static_class();

	bool operator<(const ustruct& Other) const
	{
		return true;
	}
	bool operator>(const ustruct& Other) const
	{
		return false;
	}
	bool operator>=(const ustruct& Other) const
	{
		return false;
	}
	bool operator<=(const ustruct& Other) const
	{
		return false;
	}
};

class ufunction : public ustruct
{
	using ustruct::ustruct;

public:
	EFunctionFlags get_function_flags() const;
	bool has_flags(EFunctionFlags Flags) const;

	void* get_exec_function() const;

	uproperty get_return_property() const;

	std::string stringify_Flags(const char* Seperator = ", ") const;
	std::string get_param_struct_name() const;


	static uobject static_class();

	static ufunction find_function_objects(std::string name);
};

class uclass : public ustruct
{
	using ustruct::ustruct;

public:
	EClassCastFlags get_cast_flags() const;
	std::string stringify_cast_flags() const;
	bool is_type(EClassCastFlags TypeFlag) const;
	bool has_type(uclass TypeClass) const;
	uobject get_default_object() const;

	ufunction get_function(const std::string& ClassName, const std::string& FuncName) const;

	static uobject static_class();



	bool operator<(const uclass& Other) const
	{
		return true;
	}
	bool operator>(const uclass& Other) const
	{
		return false;
	}
	bool operator>=(const uclass& Other) const
	{
		return false;
	}
	bool operator<=(const uclass& Other) const
	{
		return false;
	}
};

class uproperty
{
protected:
	uint8* Base;

public:
	uproperty() = default;
	uproperty(const uproperty&) = default;

	uproperty(void* NewProperty)
		: Base(reinterpret_cast<uint8*>(NewProperty))
	{
	}

public:
	void* get_address();

	std::pair<uclass, ffieldclass> get_class() const;
	EClassCastFlags get_cast_flags() const;

	operator bool() const;

	bool isa(EClassCastFlags TypeFlags) const;

	fname get_fname() const;
	int32 get_array_dim() const;
	int32 get_size() const;
	int32 get_offset() const;
	EPropertyFlags get_property_flags() const;
	bool has_property_flags(EPropertyFlags PropertyFlag) const;
	bool is_type(EClassCastFlags PossibleTypes) const;



	int32 get_alignment() const;

	std::string get_name() const;
	std::string get_valid_name() const;

	std::string get_cpp_type() const;



	std::string stringify_flags() const;

	uint32_t property_size() const;
	int32 get_mask()
	{
		std::uint32_t m_mask = (std::uint32_t)property_size();
		static_cast<char*>(static_cast<void*>(&m_mask));
		return static_cast<char*>(static_cast<void*>(&m_mask))[3];
	}

	static uobject static_class();
public:
	template<typename utype>
	utype cast()
	{
		return utype(Base);
	}

	template<typename utype>
	const utype cast() const
	{
		return utype(Base);
	}
};

class ubyteproperty : public uproperty
{
	using uproperty::uproperty;

public:
	uenum get_enum() const;

	std::string get_cpp_type() const;
};

class uboolproperty : public uproperty
{
	using uproperty::uproperty;

public:
	uint8 GetFieldMask() const;
	uint8 GetBitIndex() const;
	bool IsNativeBool() const;

	std::string GetCppType() const;
};

class uobjectproperty : public uproperty
{
	using uproperty::uproperty;

public:
	uclass get_property_class() const;

	std::string get_cpp_type() const;
};

class uclassproperty : public uobjectproperty
{
	using uobjectproperty::uobjectproperty;

public:
	uclass get_meta_class() const;

	std::string get_cpp_type() const;
};

class uweakobjectproperty : public uobjectproperty
{
	using uobjectproperty::uobjectproperty;

public:
	std::string get_cpp_type() const;
};

class ulazyobjectproperty : public uobjectproperty
{
	using uobjectproperty::uobjectproperty;

public:
	std::string get_cpp_type() const;
};

class usoftobjectproperty : public uobjectproperty
{
	using uobjectproperty::uobjectproperty;

public:
	std::string get_cpp_type() const;
};

class UESoftClassProperty : public uclassproperty
{
	using uclassproperty::uclassproperty;

public:
	std::string get_cpp_type() const;
};

class uinterfaceproperty : public uobjectproperty
{
	using uobjectproperty::uobjectproperty;

public:
	std::string get_cpp_type() const;
};

class ustructproperty : public uproperty
{
	using uproperty::uproperty;

public:
	ustruct get_underlaying_struct() const;

	std::string get_cpp_type() const;
};

class uarrayproperty : public uproperty
{
	using uproperty::uproperty;

public:
	uproperty get_inner_property() const;

	std::string get_cpp_type() const;
};

class udelegateproperty : public uproperty
{
	using uproperty::uproperty;

public:
	ufunction get_signature_function() const;

	std::string get_cpp_type() const;
};

class umapproperty : public uproperty
{
	using uproperty::uproperty;

public:
	uproperty get_key_property() const;
	uproperty get_value_property() const;

	std::string get_cpp_type() const;
};

class usetproperty : public uproperty
{
	using uproperty::uproperty;

public:
	uproperty get_element_property() const;

	std::string get_cpp_type() const;
};

class uenumproperty : public uproperty
{
	using uproperty::uproperty;

public:
	uproperty get_underlaying_property() const;
	uenum get_enum() const;

	std::string get_cpp_type() const;
};

class UEFieldPathProperty : public uproperty
{
	using uproperty::uproperty;

public:
	ffieldclass get_field_class() const;

	std::string get_cpp_type() const;
};

class UEOptionalProperty : public uproperty
{
	using uproperty::uproperty;

public:
	uproperty get_value_property() const;

	std::string get_cpp_type() const;
};

