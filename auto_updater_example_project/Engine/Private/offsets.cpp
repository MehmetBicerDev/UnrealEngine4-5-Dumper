#include "offsets.h"
#include "object_array.h"
#include "name_array.h"
#include "unreal_objects.h"
#include "util.h"
#include "offset_finder.h"

#include "unreal_types.h"
#include "name_array.h"

void offsets::process_event::init_pe()
{
	void** vft = *(void***)object_array::get_by_index(0).get_address();

	auto is_process_event = [](const uint8_t* FuncAddress, [[maybe_unused]] int32_t Index) -> bool
		{
			return FindPatternInRange({ 0xF7, -0x1, ufunctiono::function_flags, 0x0, 0x0, 0x0, 0x0, 0x04, 0x0, 0x0 }, FuncAddress, 0x400)
				&& FindPatternInRange({ 0xF7, -0x1, ufunctiono::function_flags, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x0 }, FuncAddress, 0xF00);
		};

	const void* process_event_addr = nullptr;
	int32_t process_event_idx = 0;

	auto [FuncPtr, FuncIdx] = IterateVTableFunctions(vft, is_process_event);

	process_event_addr = FuncPtr;
	process_event_idx = FuncIdx;


	if (!FuncPtr)
	{
		void* possible_pe_addr = (void*)FindByWStringInAllSections(L"Accessed None").FindNextFunctionStart();

		auto is_same_addr = [possible_pe_addr](const uint8_t* func_address, [[maybe_unused]] int32_t index) -> bool
			{
				return func_address == possible_pe_addr;
			};

		auto [FuncPtr2, FuncIdx2] = IterateVTableFunctions(vft, is_same_addr);

		process_event_addr = FuncPtr2;
		process_event_idx = FuncIdx2;
	}


	if (process_event_addr)
	{
		pe_index = process_event_idx;
		pe_offset = GetOffset(process_event_addr);
	}
}


void offsets::process_event::init_pe(int32_t index)
{
	pe_index = index;

	void** vft = *reinterpret_cast<void***>(object_array::get_by_index(0).get_address());

	uintptr_t image_base = GetImageBase();

	pe_offset = uintptr_t(vft[pe_index]) - image_base;
}

void offsets::world::init_world()
{
	uclass world = object_array::find_class_fast("World");

	for (uobject obj : object_array())
	{
		if (obj.has_any_flags(EObjectFlags::ClassDefaultObject) || !obj.isa(world))
			continue;


		void* result = FindAlignedValueInProcess(obj.get_address());

		if (result)
		{
			world_offset = GetOffset(result);
		}
	}
}

void offsets::text::init_text_offsets()
{
	if (!offsets::process_event::pe_index) return;

	auto is_valid_ptr = [](void* a) -> bool
		{
			return !IsBadReadPtr(a) && (uintptr_t(a) & 0x1) == 0;
		};

	ufunction conv_stringtotext = object_array::find_object_fast<ufunction>("Conv_StringToText", EClassCastFlags::Function);

	uproperty instring_prop = nullptr;
	uproperty return_prop = nullptr;

	for (uproperty prop : conv_stringtotext.get_properties())
	{
		(prop.has_property_flags(EPropertyFlags::ReturnParm) ? return_prop : instring_prop) = prop;
	}

	const int32_t param_size = conv_stringtotext.get_struct_size();
	const int32_t ftext_size = return_prop.get_size();

	const int32_t string_offset = instring_prop.get_offset();
	const int32_t return_value_offset = return_prop.get_offset();


	text::text_size = ftext_size;
#pragma warning(disable: 6255)

	uint8_t* param_ptr = static_cast<uint8_t*>(alloca(param_size));
	memset(param_ptr, 0, param_size);

	constexpr const wchar_t* string_text = L"ThisIsAGoodString!";
	constexpr int32_t string_length = (sizeof(L"ThisIsAGoodString!") / sizeof(wchar_t));
	constexpr int32_t string_length_bytes = (sizeof(L"ThisIsAGoodString!"));

	*reinterpret_cast<fstring*>(param_ptr + string_offset) = string_text;


	object_array::get_by_index(0).process_event(conv_stringtotext, param_ptr);

	uint8_t* ftext_data_ptr = nullptr;


	for (int32_t i = 0; i < (ftext_size - sizeof(void*)); i += sizeof(void*))
	{
		void* possible_text_data_ptr = *reinterpret_cast<void**>(param_ptr + return_value_offset + i);

		if (is_valid_ptr(possible_text_data_ptr))
		{
			ftext_data_ptr = static_cast<uint8_t*>(possible_text_data_ptr);

			text::text_dat_offset = i;
			break;
		}
	}

	if (!ftext_data_ptr) return;

	constexpr int32_t max_offset = 0x50;
	constexpr int32_t start_offset = sizeof(void*);

	for (int32_t i = start_offset; i < max_offset; i += sizeof(int32_t))
	{
		wchar_t* possible_string_ptr = *reinterpret_cast<wchar_t**>((ftext_data_ptr + i) - 0x08);
		const int32_t possible_length = *reinterpret_cast<int32_t*>(ftext_data_ptr + i);

		if (possible_length == string_length && possible_string_ptr && is_valid_ptr(possible_string_ptr) && memcmp(string_text, possible_string_ptr, string_length_bytes) == 0)
		{
			text::in_text_data_string_offset = (i - 0x08);
		}
	}
}


void offsets::init()
{
	offset_finder::init_uobject_offsets();

	offset_finder::init_fname_settings();

	::name_array::post_init();

	offsets::ustructo::children = offset_finder::find_child_offset();
	std::cout << std::format("offsets::ustructo::Children: 0x{:X}\n", offsets::ustructo::children);

	offsets::ufieldo::next = offset_finder::find_ufield_next_offset();
	std::cout << std::format("Off::Field::Next: 0x{:X}\n", offsets::ufieldo::next);

	offsets::ustructo::super_struct = offset_finder::find_super_offset();
	std::cout << std::format("offsets::ustructo::SuperStruct: 0x{:X}\n", offsets::ustructo::super_struct);

	offsets::ustructo::size = offset_finder::find_struct_size_offset();
	std::cout << std::format("offsets::ustructo::Size: 0x{:X}\n", offsets::ustructo::size);

	offsets::ustructo::min_alignment = offset_finder::find_min_alignment_offset();
	std::cout << std::format("offsets::ustructo::MinAlignemnts: 0x{:X}\n", offsets::ustructo::min_alignment);

	offsets::uclasso::cast_flags = offset_finder::find_cast_flags_offset();
	std::cout << std::format("Off::UClass::CastFlags: 0x{:X}\n", offsets::uclasso::cast_flags);

	if (Settings::Internal::bUseFProperty)
	{
		std::cout << std::format("Game uses FProperty system\n\n");

		offsets::ustructo::child_properties = offset_finder::find_child_properties_offset();
		std::cout << std::format("offsets::ustructo::ChildProperties: 0x{:X}\n", offsets::ustructo::child_properties);

		offset_finder::fixup_hardcoded_offsets(); // must be called after FindChildPropertiesOffset 

		offsets::ffield::next = offset_finder::find_ffield_next_offset();
		std::cout << std::format("Off::FField::Next: 0x{:X}\n", offsets::ffield::next);

		offsets::ffield::name = offset_finder::find_ffield_name_offset();
		std::cout << std::format("Off::FField::Name: 0x{:X}\n", offsets::ffield::name);

		/*
		* FNameSize might be wrong at this point of execution.
		* FField::Flags is not critical so a fix is only applied later in OffsetFinder::PostInitFNameSettings().
		*/
		offsets::ffield::flags = offsets::ffield::name + offsets::name::fname_size;
		std::cout << std::format("Off::FField::Flags: 0x{:X}\n", offsets::ffield::flags);
	}

	offsets::uclasso::class_default_object = offset_finder::find_default_object_offset();
	std::cout << std::format("Off::UClass::ClassDefaultObject: 0x{:X}\n", offsets::uclasso::class_default_object);

	offsets::uenumo::names = offset_finder::find_enum_names_offset();
	std::cout << std::format("Off::UEnum::Names: 0x{:X}\n", offsets::uenumo::names);

	offsets::ufunctiono::function_flags = offset_finder::find_function_flags_offset();
	std::cout << std::format("Off::UFunction::FunctionFlags: 0x{:X}\n", offsets::ufunctiono::function_flags) << std::endl;

	offsets::ufunctiono::exec_function = offset_finder::find_function_native_func_offset();
	std::cout << std::format("Off::UFunction::ExecFunction: 0x{:X}\n", offsets::ufunctiono::exec_function) << std::endl;

	offsets::fproperty::elements_size = offset_finder::find_element_size_offset();
	std::cout << std::format("Off::Property::ElementSize: 0x{:X}\n", offsets::fproperty::elements_size);

	offsets::fproperty::array_dim = offset_finder::find_array_dim_offset();
	std::cout << std::format("Off::Property::ArrayDim: 0x{:X}\n", offsets::fproperty::array_dim);

	offsets::fproperty::offset_internal = offset_finder::find_offset_interna_offset();
	std::cout << std::format("Off::Property::Offset_Internal: 0x{:X}\n", offsets::fproperty::offset_internal);

	offsets::fproperty::property_flags = offset_finder::find_property_flags_offset();
	std::cout << std::format("Off::Property::PropertyFlags: 0x{:X}\n", offsets::fproperty::property_flags);

	offsets::properties::property_size = offset_finder::find_bool_property_base_offset();
	std::cout << std::format("UPropertySize: 0x{:X}\n", offsets::properties::property_size) << std::endl;

	offsets::array_property::inner = offset_finder::find_inner_type_offset(offsets::properties::property_size);
	std::cout << std::format("Off::ArrayProperty::Inner: 0x{:X}\n", offsets::array_property::inner);

	offsets::set_property::element_prop = offset_finder::FindSetPropertyBaseOffset(offsets::properties::property_size);
	std::cout << std::format("Off::SetProperty::ElementProp: 0x{:X}\n", offsets::set_property::element_prop);

	offsets::map_property::base = offset_finder::FindMapPropertyBaseOffset(offsets::properties::property_size);
	std::cout << std::format("Off::MapProperty::Base: 0x{:X}\n", offsets::map_property::base) << std::endl;

	offsets::ulevelo::actors = offset_finder::find_level_actors_offset();
	std::cout << std::format("Off::ULevel::Actors: 0x{:X}\n", offsets::ulevelo::actors) << std::endl;

	offset_finder::post_init_fname_settings();

	offsets::byte_property::senum = offsets::properties::property_size;
	offsets::bool_property::base = offsets::properties::property_size;
	offsets::object_property::property_class = offsets::properties::property_size;
	offsets::struct_property::sstruct = offsets::properties::property_size;
	offsets::enum_property::base = offsets::properties::property_size;
	offsets::delegate_property::signature_function = offsets::properties::property_size;
	offsets::field_path_property::field_class = offsets::properties::property_size;
	offsets::optional_property::value_property = offsets::properties::property_size;

	offsets::class_property::meta_class = offsets::properties::property_size + 0x8; //0x8 inheritance from ObjectProperty
}

void offsets::init_engine()
{
	object_array::init();
	::fname::init();
	offsets::init();
	offsets::process_event::init_pe(); //Must be at this position, relies on offsets initialized in Off::Init()

	offsets::world::init_world(); //Must be at this position, relies on offsets initialized in Off::Init()

	offsets::text::init_text_offsets(); //Must be at this position, relies on offsets initialized in Off::InitPE()

	init_settings();
}

void offsets::init_settings()
{
	init_weakobjectptr_settings();
	init_largeworld_coordinate_settings();
}

void offsets::init_weakobjectptr_settings()
{
	ustruct LoadAsset = object_array::find_object_fast<ufunction>("LoadAsset", EClassCastFlags::Function);

	if (!LoadAsset)
	{
		std::cout << "\nDumper-7: 'LoadAsset' wasn't found, could not determine value for 'bIsWeakObjectPtrWithoutTag'!\n" << std::endl;
		return;
	}

	uproperty Asset = LoadAsset.find_member("Asset", EClassCastFlags::SoftObjectProperty);
	if (!Asset)
	{
		std::cout << "\nDumper-7: 'Asset' wasn't found, could not determine value for 'bIsWeakObjectPtrWithoutTag'!\n" << std::endl;
		return;
	}

	ustruct SoftObjectPath = object_array::find_object_fast<ustruct>("SoftObjectPath");

	constexpr int32 SizeOfFFWeakObjectPtr = 0x08;
	constexpr int32 OldUnrealAssetPtrSize = 0x10;
	const int32 SizeOfSoftObjectPath = SoftObjectPath ? SoftObjectPath.get_struct_size() : OldUnrealAssetPtrSize;

	Settings::Internal::bIsWeakObjectPtrWithoutTag = Asset.get_size() <= (SizeOfSoftObjectPath + SizeOfFFWeakObjectPtr);
}

void offsets::init_largeworld_coordinate_settings()
{
	ustruct FVectorStruct = object_array::find_object_fast<ustruct>("Vector", EClassCastFlags::Struct);

	if (!FVectorStruct) [[unlikely]]
	{
		std::cout << "\nSomething went horribly wrong, FVector wasn't even found!\n\n" << std::endl;
		return;
	}

	uproperty XProperty = FVectorStruct.find_member("X");

	if (!XProperty) [[unlikely]]
	{
		std::cout << "\nSomething went horribly wrong, FVector::X wasn't even found!\n\n" << std::endl;
		return;
	}

	Settings::Internal::bUseLargeWorldCoordinates = XProperty.isa(EClassCastFlags::DoubleProperty);

}
