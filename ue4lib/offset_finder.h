#include <vector>
#include "object_array.h"
#include "offsets.h"
#include "util.h"
#include "settings.h"
#include "unreal_objects.h"
#include "unreal_types.h"

namespace offset_finder
{
    constexpr int32_t offset_not_found = -1;

    template<int alignment = 4, typename T>
    inline int32_t find_offset(std::vector<std::pair<void*, T>>& object_value_pair, int min_offset = 0x28, int max_offset = 0x1A0)
    {
        int32_t highest_found_offset = min_offset;

        for (int i = 0; i < object_value_pair.size(); i++)
        {
            uint8_t* byte_ptr = (uint8_t*)(object_value_pair[i].first);

            for (int j = highest_found_offset; j < max_offset; j += alignment)
            {
                if ((*reinterpret_cast<T*>(byte_ptr + j)) == object_value_pair[i].second && j >= highest_found_offset)
                {
                    if (j > highest_found_offset)
                    {
                        highest_found_offset = j;
                        i = 0;
                    }
                    j = max_offset;
                }
            }
        }

        return highest_found_offset != min_offset ? highest_found_offset : offset_not_found;
    }

    template<bool check_for_vft = true>
    inline int32_t get_valid_pointer_offset(uint8_t* obj_a, uint8_t* obj_b, int32_t starting_offset, int32_t max_offset)
    {
        if (IsBadReadPtr(obj_a) || IsBadReadPtr(obj_b))
            return offset_not_found;

        for (int j = starting_offset; j <= max_offset; j += 0x8)
        {
            const bool is_a_valid = !IsBadReadPtr(*reinterpret_cast<void**>(obj_a + j)) && (check_for_vft ? !IsBadReadPtr(**reinterpret_cast<void***>(obj_a + j)) : true);
            const bool is_b_valid = !IsBadReadPtr(*reinterpret_cast<void**>(obj_b + j)) && (check_for_vft ? !IsBadReadPtr(**reinterpret_cast<void***>(obj_b + j)) : true);

            if (is_a_valid && is_b_valid)
                return j;
        }

        return offset_not_found;
    };

    /* UObject */
    inline void init_uobject_offsets()
    {

        uint8_t* obj_a = (uint8_t*)object_array::get_by_index(0x55).get_address();
        uint8_t* obj_b = (uint8_t*)object_array::get_by_index(0x123).get_address();

        auto get_index_offset = [&obj_a, &obj_b]() -> int32_t
            {
                std::vector<std::pair<void*, int32_t>> infos;

                infos.emplace_back(object_array::get_by_index(0x055).get_address(), 0x055);
                infos.emplace_back(object_array::get_by_index(0x123).get_address(), 0x123);

                return find_offset<4>(infos, 0x0);
            };

        offsets::uobjecto::vft = 0x00;
        offsets::uobjecto::flags = sizeof(void*);
        offsets::uobjecto::index = get_index_offset();
        offsets::uobjecto::klass = get_valid_pointer_offset(obj_a, obj_b, offsets::uobjecto::index + sizeof(int), 0x40);
        offsets::uobjecto::name = offsets::uobjecto::klass + sizeof(void*);
        offsets::uobjecto::outer = get_valid_pointer_offset(obj_a, obj_b, offsets::uobjecto::name + 0x8, 0x40);
		std::cout << std::format("offsets::uobjecto::outer: 0x{:X}\n", offsets::uobjecto::outer);

        // loop a few times in case we accidentally choose a UPackage (which doesn't have an Outer) to find Outer
        while (offsets::uobjecto::outer == offset_not_found)
        {
            obj_a = (uint8_t*)object_array::get_by_index(rand() % 0x400).get_address();
            obj_b = (uint8_t*)object_array::get_by_index(rand() % 0x400).get_address();

            offsets::uobjecto::outer = get_valid_pointer_offset(obj_a, obj_b, offsets::uobjecto::name + 0x8, 0x40);
        }
    }

    inline void fixup_hardcoded_offsets()
    {
        if (Settings::Internal::bUseCasePreservingName)
        {
            offsets::ffield::flags += 0x8;

            offsets::ffieldclass::id += 0x08;
            offsets::ffieldclass::cast_flags += 0x08;
            offsets::ffieldclass::class_flags += 0x08;
            offsets::ffieldclass::super_class += 0x08;
        }

        if (Settings::Internal::bUseFProperty)
        {
            /*
            * On versions below 5.1.1: class FFieldVariant { void*, bool } -> extends to { void*, bool, uint8[0x7] }
            * ON versions since 5.1.1: class FFieldVariant { void* }
            *
            * Check:
            * if FFieldVariant contains a bool, the memory at the bools offset will not be a valid pointer
            * if FFieldVariant doesn't contain a bool, the memory at the bools offset will be the next member of FField, the Next ptr [valid]
            */

            void* possible_next_ptr_or_bool0 = *(void**)((uint8_t*)object_array::find_class_fast("Actor").get_child_properties().get_address() + 0x18);
            void* possible_next_ptr_or_bool1 = *(void**)((uint8_t*)object_array::find_class_fast("Actor").get_child_properties().get_address() + 0x20);
            void* possible_next_ptr_or_bool2 = *(void**)((uint8_t*)object_array::find_class_fast("Pawn").get_child_properties().get_address() + 0x18);
            auto is_valid_ptr = [](void* a) -> bool
                {
                    return !IsBadReadPtr(a) && (uintptr_t(a) & 0x1) == 0; // realistically, there wont be any pointers to unaligned memory
                };
            if (is_valid_ptr(possible_next_ptr_or_bool0) && is_valid_ptr(possible_next_ptr_or_bool1) && is_valid_ptr(possible_next_ptr_or_bool2))
            {
                Settings::Internal::bUseMaskForFieldOwner = true;

                offsets::ffield::next -= 0x08;
                offsets::ffield::name -= 0x08;
                offsets::ffield::flags -= 0x08;
            }
            
        }
    }

    inline void init_fname_settings()
    {
        uobject first_object = object_array::get_by_index(0);

        const uint8_t* name_address = static_cast<const uint8_t*>(first_object.get_fname().get_address());
        const int32_t fname_first_int = *reinterpret_cast<const int32_t*>(name_address);
        const int32_t fname_second_int = *reinterpret_cast<const int32_t*>(name_address + 0x04);
        const int32_t fname_size = offsets::uobjecto::outer - offsets::uobjecto::name;

        offsets::fname::comp_idx = 0x0;
        offsets::fname::number = 0x04;

        auto get_num_names_with_number_one_to_four = []() -> int32_t
            {
                int32_t names_with_number_one_to_four = 0x0;

                for (uobject obj : object_array())
                {
                    const int32_t number = obj.get_fname().get_number();

                    if (number > 0x0 && number < 0x5)
                        names_with_number_one_to_four++;
                }

                return names_with_number_one_to_four;
            };

        constexpr float min_percentage = 0x03f;

        const int32_t fname_number_threashold = (object_array::num() * min_percentage);

        offsets::fname::comp_idx = 0x0;

        if (fname_size == 0x8 && fname_first_int == fname_second_int)
        {
            Settings::Internal::bUseCasePreservingName = true;
            Settings::Internal::bUseUoutlineNumberName = true;


            offsets::fname::number = -0x1;
            offsets::name::fname_size = 0x8;
        }
        else if (fname_size > 0x8)
        {
            Settings::Internal::bUseUoutlineNumberName = false;
            Settings::Internal::bUseCasePreservingName = true;

            offsets::fname::number = fname_first_int == fname_second_int ? 0x8 : 0x4;

            offsets::name::fname_size = 0xC;
        }
        else if (fname_size == 0x4)
        {
            Settings::Internal::bUseUoutlineNumberName = true;
            Settings::Internal::bUseCasePreservingName = false;

            offsets::fname::number = -0x1;

            offsets::name::fname_size = 0x4;
        }
        else /* Default */
        {
            Settings::Internal::bUseUoutlineNumberName = false;
            Settings::Internal::bUseCasePreservingName = false;

            offsets::fname::number = 0x4;
            offsets::name::fname_size = 0x8;
        }
    }

    inline int32_t find_ufield_next_offset()
    {
        uint8_t* kismet_system_library_child = reinterpret_cast<uint8_t*>(object_array::find_object_fast<ustruct>("KismetSystemLibrary").get_child().get_address());
        uint8_t* kismet_string_library_child = reinterpret_cast<uint8_t*>(object_array::find_object_fast<ustruct>("KismetStringLibrary").get_child().get_address());

        return get_valid_pointer_offset(kismet_system_library_child, kismet_string_library_child, offsets::uobjecto::outer + 0x08, 0x48);
    }


	inline int32_t find_ffield_next_offset()
	{
		uint8_t* guid_children = reinterpret_cast<uint8_t*>(object_array::find_object_fast<ustruct>("Guid").get_child_properties().get_address());
		uint8_t* vector_children = reinterpret_cast<uint8_t*>(object_array::find_object_fast<ustruct>("Vector").get_child_properties().get_address());

		return get_valid_pointer_offset(guid_children, vector_children, offsets::ffield::owner + 0x8, 0x48);
	}

	inline int32_t find_ffield_name_offset()
	{
		ffield guid_child = object_array::find_object_fast<ustruct>("Guid").get_child_properties();
		ffield vector_child = object_array::find_object_fast<ustruct>("Vector").get_child_properties();

		std::string guid_child_name = guid_child.get_name();
		std::string vector_child_name = vector_child.get_name();

		if ((guid_child_name == "A" || guid_child_name == "D") && (vector_child_name == "X" || vector_child_name == "Z"))
			return offsets::ffield::name;

		for (offsets::ffield::name = offsets::ffield::owner; offsets::ffield::name < 0x40; offsets::ffield::name += 4)
		{
			guid_child_name = guid_child.get_name();
			vector_child_name = vector_child.get_name();

			if ((guid_child_name == "A" || guid_child_name == "D") && (vector_child_name == "X" || vector_child_name == "Z"))
				return offsets::ffield::name;
		}

		return offset_not_found;
	}

	/* UEnum */
	inline int32_t find_enum_names_offset()
	{
		std::vector<std::pair<void*, int32_t>> infos;

		infos.push_back({ object_array::find_object_fast("ENetRole").get_address(), 0x5 });
		infos.push_back({ object_array::find_object_fast("ETraceTypeQuery").get_address(), 0x22 });

		int ret = find_offset(infos) - 0x8;

		struct Name08Byte { uint8_t Pad[0x08]; };
		struct Name16Byte { uint8_t Pad[0x10]; };

		uint8_t* array_address = static_cast<uint8_t*>(infos[0].first) + ret;

		if (Settings::Internal::bUseCasePreservingName)
		{
			tarray<tpair<Name16Byte, int64_t>>& array_of_name_value_pairs = *reinterpret_cast<tarray<tpair<Name16Byte, int64_t>>*>(array_address);

			if (array_of_name_value_pairs[1].Second != 1)
				Settings::Internal::bIsEnumNameOnly = true;


		}
		else
		{
			if (reinterpret_cast<tarray<tpair<Name08Byte, int64_t>>*>(static_cast<uint8_t*>(infos[0].first) + ret)->operator[](1).Second != 1)
				Settings::Internal::bIsEnumNameOnly = true;
		}

		return ret;
	}

	/* UStruct */
	inline int32_t find_super_offset()
	{
		std::vector<std::pair<void*, void*>> infos;

		infos.push_back({ object_array::find_object_fast("Struct").get_address(), object_array::find_object_fast("Field").get_address() });
		infos.push_back({ object_array::find_object_fast("Class").get_address(), object_array::find_object_fast("Struct").get_address() });

		// Thanks to the ue4 dev who decided UStruct should be spelled Ustruct
		if (infos[0].first == nullptr)
			infos[0].first = infos[1].second = object_array::find_object_fast("struct").get_address();

		return find_offset(infos);
	}

	inline int32_t find_child_offset()
	{
		std::vector<std::pair<void*, void*>> Infos;

		Infos.push_back({ object_array::find_object_fast("PlayerController").get_address(), object_array::find_object_fast_in_outer("WasInputKeyJustReleased", "PlayerController").get_address() });
		Infos.push_back({ object_array::find_object_fast("Controller").get_address(), object_array::find_object_fast_in_outer("UnPossess", "Controller").get_address() });

		if (find_offset(Infos) == offset_not_found)
		{
			Infos.clear();

			Infos.push_back({ object_array::find_object_fast("Vector").get_address(), object_array::find_object_fast_in_outer("X", "Vector").get_address() });
			Infos.push_back({ object_array::find_object_fast("Vector4").get_address(), object_array::find_object_fast_in_outer("X", "Vector4").get_address() });
			Infos.push_back({ object_array::find_object_fast("Vector2D").get_address(), object_array::find_object_fast_in_outer("X", "Vector2D").get_address() });
			Infos.push_back({ object_array::find_object_fast("Guid").get_address(), object_array::find_object_fast_in_outer("A","Guid").get_address() });

			return find_offset(Infos);
		}

		Settings::Internal::bUseFProperty = true;

		return find_offset(Infos);
	}

	inline int32_t find_child_properties_offset()
	{
		uint8_t* ObjA = (uint8_t*)object_array::find_object_fast("Color").get_address();
		uint8_t* ObjB = (uint8_t*)object_array::find_object_fast("Guid").get_address();

		return get_valid_pointer_offset(ObjA, ObjB, offsets::ustructo::children + 0x08, 0x80);
	}

	inline int32_t find_struct_size_offset()
	{
		std::vector<std::pair<void*, int32_t>> Infos;

		Infos.push_back({ object_array::find_object_fast("Color").get_address(), 0x04 });
		Infos.push_back({ object_array::find_object_fast("Guid").get_address(), 0x10 });

		return find_offset(Infos);
	}

	inline int32_t find_min_alignment_offset()
	{
		std::vector<std::pair<void*, int32_t>> Infos;

		Infos.push_back({ object_array::find_object_fast("Transform").get_address(), 0x10 });
		Infos.push_back({ object_array::find_object_fast("PlayerController").get_address(), 0x8 });

		return find_offset(Infos);
	}

	/* UFunction */
	inline int32_t find_function_flags_offset()
	{
		std::vector<std::pair<void*, EFunctionFlags>> Infos;

		Infos.push_back({ object_array::find_object_fast("WasInputKeyJustPressed").get_address(), EFunctionFlags::Final | EFunctionFlags::Native | EFunctionFlags::Public | EFunctionFlags::BlueprintCallable | EFunctionFlags::BlueprintPure | EFunctionFlags::Const });
		Infos.push_back({ object_array::find_object_fast("ToggleSpeaking").get_address(), EFunctionFlags::Exec | EFunctionFlags::Native | EFunctionFlags::Public });
		Infos.push_back({ object_array::find_object_fast("SwitchLevel").get_address(), EFunctionFlags::Exec | EFunctionFlags::Native | EFunctionFlags::Public });

		int32_t Ret = find_offset(Infos);

		if (Ret == offset_not_found)
		{
			for (auto& [_, Flags] : Infos)
				Flags = Flags | EFunctionFlags::RequiredAPI;
		}

		return find_offset(Infos);
	}

	inline int32_t find_function_native_func_offset()
	{
		std::vector<std::pair<void*, EFunctionFlags>> Infos;

		uintptr_t WasInputKeyJustPressed = reinterpret_cast<uintptr_t>(object_array::find_object_fast("WasInputKeyJustPressed").get_address());
		uintptr_t ToggleSpeaking = reinterpret_cast<uintptr_t>(object_array::find_object_fast("ToggleSpeaking").get_address());
		uintptr_t SwitchLevel = reinterpret_cast<uintptr_t>(object_array::find_object_fast("SwitchLevel").get_address());

		for (int i = 0x40; i < 0x140; i += 8)
		{
			if (IsInProcessRange(*reinterpret_cast<uintptr_t*>(WasInputKeyJustPressed + i)) && IsInProcessRange(*reinterpret_cast<uintptr_t*>(ToggleSpeaking + i)) && IsInProcessRange(*reinterpret_cast<uintptr_t*>(SwitchLevel + i)))
				return i;
		}

		return 0x0;
	}

	inline int32_t find_num_params_offset()
	{
		std::vector<std::pair<void*, uint8_t>> Infos;

		Infos.push_back({ object_array::find_object_fast("SwitchLevel").get_address(), 0x1 });
		Infos.push_back({ object_array::find_object_fast("SetViewTargetWithBlend").get_address(), 0x5 });
		Infos.push_back({ object_array::find_object_fast("SetHapticsByValue").get_address(), 0x3 });
		Infos.push_back({ object_array::find_object_fast("SphereTraceSingleForObjects").get_address(), 0xE });

		return find_offset<1>(Infos);
	}

	inline int32_t find_param_size_offset()
	{
		std::vector<std::pair<void*, uint16_t>> Infos;

		// TODO (encryqed) : Fix it anyways somehow, for some reason its one byte off? Idk why i just add a byte manually
		Infos.push_back({ object_array::find_object_fast("SwitchLevel").get_address(), 0x10 });
		Infos.push_back({ object_array::find_object_fast("SetViewTargetWithBlend").get_address(), 0x15 });
		Infos.push_back({ object_array::find_object_fast("SetHapticsByValue").get_address(), 0x9 });
		Infos.push_back({ object_array::find_object_fast("SphereTraceSingleForObjects").get_address(), 0x109 });

		int ParamSizeOffset = find_offset<1>(Infos);

		if (Settings::Internal::bUseFProperty)
		{
			return ParamSizeOffset + 0x1;
		}

		return ParamSizeOffset;
	}

	/* UClass */
	inline int32_t find_cast_flags_offset()
	{
		std::vector<std::pair<void*, EClassCastFlags>> Infos;

		Infos.push_back({ object_array::find_object_fast("Actor").get_address(), EClassCastFlags::Actor });
		Infos.push_back({ object_array::find_object_fast("Class").get_address(), EClassCastFlags::Field | EClassCastFlags::Struct | EClassCastFlags::Class });

		return find_offset(Infos);
	}

	inline int32_t find_default_object_offset()
	{
		std::vector<std::pair<void*, void*>> Infos;

		Infos.push_back({ object_array::find_object_fast("Object").get_address(), object_array::find_object_fast("Default__Object").get_address() });
		Infos.push_back({ object_array::find_object_fast("Field").get_address(), object_array::find_object_fast("Default__Field").get_address() });

		return find_offset(Infos);
	}

	/* Property */
	inline int32_t find_element_size_offset()
	{
		std::vector<std::pair<void*, int32_t>> Infos;

		ustruct Guid = object_array::find_object_fast("Guid", EClassCastFlags::Struct).cast<ustruct>();

		Infos.push_back({ Guid.find_member("A").get_address(), 0x04 });
		Infos.push_back({ Guid.find_member("B").get_address(), 0x04 });
		Infos.push_back({ Guid.find_member("C").get_address(), 0x04 });

		return find_offset(Infos);
	}

	inline int32_t find_array_dim_offset()
	{
		std::vector<std::pair<void*, int32_t>> Infos;

		ustruct Guid = object_array::find_object_fast("Guid", EClassCastFlags::Struct).cast<ustruct>();

		Infos.push_back({ Guid.find_member("A").get_address(), 0x01 });
		Infos.push_back({ Guid.find_member("B").get_address(), 0x01 });
		Infos.push_back({ Guid.find_member("C").get_address(), 0x01 });

		const int32_t MinOffset = offsets::fproperty::elements_size - 0x10;
		const int32_t MaxOffset = offsets::fproperty::elements_size + 0x10;

		return find_offset(Infos, MinOffset, MaxOffset);
	}

	inline int32_t find_property_flags_offset()
	{
		std::vector<std::pair<void*, EPropertyFlags>> Infos;


		ustruct Guid = object_array::find_object_fast("Guid", EClassCastFlags::Struct).cast<ustruct>();
		ustruct Color = object_array::find_object_fast("Color", EClassCastFlags::Struct).cast<ustruct>();

		constexpr EPropertyFlags GuidMemberFlags = EPropertyFlags::Edit | EPropertyFlags::ZeroConstructor | EPropertyFlags::SaveGame | EPropertyFlags::IsPlainOldData | EPropertyFlags::NoDestructor | EPropertyFlags::HasGetValueTypeHash;
		constexpr EPropertyFlags ColorMemberFlags = EPropertyFlags::Edit | EPropertyFlags::BlueprintVisible | EPropertyFlags::ZeroConstructor | EPropertyFlags::SaveGame | EPropertyFlags::IsPlainOldData | EPropertyFlags::NoDestructor | EPropertyFlags::HasGetValueTypeHash;

		Infos.push_back({ Guid.find_member("A").get_address(), GuidMemberFlags });
		Infos.push_back({ Color.find_member("R").get_address(), ColorMemberFlags });

		int FlagsOffset = find_offset(Infos);

		// Same flags without AccessSpecifier
		if (FlagsOffset == offset_not_found)
		{
			Infos[0].second |= EPropertyFlags::NativeAccessSpecifierPublic;
			Infos[1].second |= EPropertyFlags::NativeAccessSpecifierPublic;

			FlagsOffset = find_offset(Infos);
		}

		return FlagsOffset;
	}

	inline int32_t find_offset_interna_offset()
	{
		std::vector<std::pair<void*, int32_t>> Infos;

		ustruct Color = object_array::find_object_fast("Color", EClassCastFlags::Struct).cast<ustruct>();

		Infos.push_back({ Color.find_member("B").get_address(), 0x00 });
		Infos.push_back({ Color.find_member("G").get_address(), 0x01 });
		Infos.push_back({ Color.find_member("R").get_address(), 0x02 });

		return find_offset(Infos);
	}

	/* BoolProperty */
	inline int32_t find_bool_property_base_offset()
	{
		std::vector<std::pair<void*, uint8_t>> Infos;

		uclass Engine = object_array::find_class_fast("Engine");
		Infos.push_back({ Engine.find_member("bIsOverridingSelectedColor").get_address(), 0xFF });
		Infos.push_back({ Engine.find_member("bEnableOnScreenDebugMessagesDisplay").get_address(), 0b00000010 });
		Infos.push_back({ object_array::find_class_fast("PlayerController").find_member("bAutoManageActiveCameraTarget").get_address(), 0xFF });

		return (find_offset<1>(Infos, offsets::fproperty::offset_internal) - 0x3);
	}

	/* ArrayProperty */
	inline int32_t find_inner_type_offset(const int32_t PropertySize)
	{
		if (!Settings::Internal::bUseFProperty)
			return PropertySize;

		if (uproperty Property = object_array::find_class_fast("GameViewportClient").find_member("DebugProperties", EClassCastFlags::ArrayProperty))
		{
			void* AddressToCheck = *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(Property.get_address()) + PropertySize);

			if (IsBadReadPtr(AddressToCheck))
				return PropertySize + 0x8;
		}

		return PropertySize;
	}

	/* SetProperty */
	inline int32_t FindSetPropertyBaseOffset(const int32_t PropertySize)
	{
		if (!Settings::Internal::bUseFProperty)
			return PropertySize;

		if (auto Object = object_array::find_object_fast<ustruct>("LevelCollection", EClassCastFlags::Struct).find_member("Levels", EClassCastFlags::SetProperty))
		{
			void* AddressToCheck = *(void**)((uint8_t*)Object.get_address() + PropertySize);

			if (IsBadReadPtr(AddressToCheck))
				return PropertySize + 0x8;
		}

		return PropertySize;
	}

	/* MapProperty */
	inline int32_t FindMapPropertyBaseOffset(const int32_t property_size)
	{
		if (!Settings::Internal::bUseFProperty)
			return property_size;

		if (auto Object = object_array::find_class_fast("UserDefinedEnum").find_member("DisplayNameMap", EClassCastFlags::MapProperty))
		{
			void* AddressToCheck = *(void**)((uint8_t*)Object.get_address() + property_size);

			if (IsBadReadPtr(AddressToCheck))
				return property_size + 0x8;
		}

		return property_size;
	}

	/* ULevel */
	inline int32_t find_level_actors_offset()
	{
		uintptr_t Lvl = 0x0;

		for (auto Obj : object_array())
		{
			if (Obj.has_any_flags(EObjectFlags::ClassDefaultObject) || !Obj.isa(EClassCastFlags::Level))
				continue;

			Lvl = reinterpret_cast<uintptr_t>(Obj.get_address());
			break;
		}

		if (Lvl == 0x0)
			return offset_not_found;

		/*
		class ULevel : public UObject
		{
			FURL URL;
			TArray<AActor*> Actors;
			TArray<AActor*> GCActors;
		};

		SearchStart = sizeof(UObject) + sizeof(FURL)
		SearchEnd = offsetof(ULevel, OwningWorld)
		*/

		int32_t SearchStart = object_array::find_class_fast("Object").get_struct_size() + object_array::find_object_fast<ustruct>("URL", EClassCastFlags::Struct).get_struct_size();
		int32_t SearchEnd = object_array::find_class_fast("Level").find_member("OwningWorld").get_offset();

		for (int i = SearchStart; i <= (SearchEnd - 0x10); i += 8)
		{
			const tarray<void*>& ActorArray = *reinterpret_cast<tarray<void*>*>(Lvl + i);

			if (ActorArray.IsValid() && !IsBadReadPtr(ActorArray.GetDataPtr()))
			{
				return i;
			}
		}

		return offset_not_found;
	}

	inline void post_init_fname_settings()
	{
		uclass PlayerStart = object_array::find_class_fast("PlayerStart");

		const int32 FNameSize = PlayerStart.find_member("PlayerStartTag").get_size();

		/* Nothing to do for us, everything is fine! */
		if (offsets::name::fname_size == FNameSize)
			return;

		/* We've used the wrong FNameSize to determine the offset of FField::Flags. Substract the old, wrong, size and add the new one.*/
		offsets::ffield::flags = (offsets::ffield::flags - offsets::name::fname_size) + FNameSize;

		const uint8* NameAddress = static_cast<const uint8*>(PlayerStart.get_fname().get_address());

		const int32 FNameFirstInt /* ComparisonIndex */ = *reinterpret_cast<const int32*>(NameAddress);
		const int32 FNameSecondInt /* [Number/DisplayIndex] */ = *reinterpret_cast<const int32*>(NameAddress + 0x4);

		if (FNameSize == 0x8 && FNameFirstInt == FNameSecondInt) /* WITH_CASE_PRESERVING_NAME + FNAME_OUTLINE_NUMBER */
		{
			Settings::Internal::bUseCasePreservingName = true;
			Settings::Internal::bUseUoutlineNumberName = true;

			offsets::fname::number = -0x1;
			offsets::name::fname_size = 0x8;
		}
		else if (FNameSize > 0x8) /* WITH_CASE_PRESERVING_NAME */
		{
			Settings::Internal::bUseUoutlineNumberName = false;
			Settings::Internal::bUseCasePreservingName = true;

			offsets::fname::number = FNameFirstInt == FNameSecondInt ? 0x8 : 0x4;

			offsets::name::fname_size = 0xC;
		}
		else if (FNameSize == 0x4) /* FNAME_OUTLINE_NUMBER */
		{
			Settings::Internal::bUseUoutlineNumberName = true;
			Settings::Internal::bUseCasePreservingName = false;

			offsets::fname::number = -0x1;

			offsets::name::fname_size = 0x4;
		}
		else /* Default */
		{
			Settings::Internal::bUseUoutlineNumberName = false;
			Settings::Internal::bUseCasePreservingName = false;

			offsets::fname::number = 0x4;
			offsets::name::fname_size = 0x8;
		}
	}
}