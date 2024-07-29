#include <format>

#include "unreal_types.h"
#include "name_array.h"

#include "settings.h"
std::string MakeNameValid(std::string&& Name)
{
	static constexpr const char* Numbers[10] =
	{
		"Zero",
		"One",
		"Two",
		"Three",
		"Four",
		"Five",
		"Six",
		"Seven",
		"Eight",
		"Nine"
	};

	if (Name == "bool")
		return "Bool";

	if (Name == "TRUE")
		return "TURR";

	if (Name == "FALSE")
		return "FLASE";

	if (Name == "NULL")
		return "NULLL";

	if (Name[0] <= '9' && Name[0] >= '0')
	{
		Name.replace(0, 1, Numbers[Name[0] - '0']);
	}
	else if ((Name[0] <= 'z' && Name[0] >= 'a') && Name[0] != 'b')
	{
		Name[0] -= 0x20;
	}

	for (int i = 0; i < Name.length(); i++)
	{
		switch (Name[i])
		{
		case '+':
			Name.replace(i, 1, "Plus");
			continue;
		case '-':
			Name.replace(i, 1, "Minus");
			continue;
		case '*':
			Name.replace(i, 1, "Star");
			continue;
		case '/':
			Name.replace(i, 1, "Slash");
			continue;
		default:
			break;
		}

		char c = Name[i];

		if (c != '_' && !((c <= 'z' && c >= 'a') || (c <= 'Z' && c >= 'A') || (c <= '9' && c >= '0')))
		{
			Name[i] = '_';
		}
	}

	return Name;
}


fname::fname(const void* Ptr)
	: Address(static_cast<const uint8*>(Ptr))
{
}

void fname::init(bool bForceGNames)
{
	constexpr std::array<const char*, 5> PossibleSigs =
	{
		"48 8D ? ? 48 8D ? ? E8",
		"48 8D ? ? ? 48 8D ? ? E8",
		"48 8D ? ? 49 8B ? E8",
		"48 8D ? ? ? 49 8B ? E8",
		"48 8D ? ? 48 8B ? E8"
		"48 8D ? ? ? 48 8B ? E8",
	};

	MemAddress StringRef = FindByStringInAllSections("ForwardShadingQuality_");

	int i = 0;
	while (!append_string && i < PossibleSigs.size())
	{
		append_string = static_cast<void(*)(const void*, fstring&)>(StringRef.RelativePattern(PossibleSigs[i], 0x50, -1 /* auto */));

		i++;
	}

	offsets::name::append_name_to_string = append_string && !bForceGNames ? GetOffset(append_string) : 0x0;

	if (!append_string || bForceGNames)
	{
		const bool bInitializedSuccessfully = name_array::try_init();

		if (bInitializedSuccessfully)
		{
			to_str = [](const void* Name) -> std::string
				{
					if (!Settings::Internal::bUseUoutlineNumberName)
					{
						const int32 Number = fname(Name).get_number();

						if (Number > 0)
							return name_array::get_name_entry(Name).get_string() + "_" + std::to_string(Number - 1);
					}

					return name_array::get_name_entry(Name).get_string();
				};

			return;
		}
		else /* Attempt to find FName::ToString as a final fallback */
		{
			/* Initialize GNames offset without committing to use GNames during the dumping process or in the SDK */
			name_array::set_gnames_without_commiting();
			fname::init_fallback();
		}
	}

	/* Initialize GNames offset without committing to use GNames during the dumping process or in the SDK */
	name_array::set_gnames_without_commiting();


	to_str = [](const void* Name) -> std::string
		{
			thread_local ffreablestring TempString(1024);

			append_string(Name, TempString);

			std::string OutputString = TempString.ToString();
			TempString.ResetNum();

			return OutputString;
		};
}

void fname::init(int32 AppendStringOffset, bool bIsToString)
{
	append_string = reinterpret_cast<void(*)(const void*, fstring&)>(GetImageBase() + AppendStringOffset);

	offsets::name::append_name_to_string = AppendStringOffset;
	offsets::name::is_using_append_string_over_to_string = !bIsToString;

	to_str = [](const void* Name) -> std::string
		{
			thread_local ffreablestring TempString(1024);

			append_string(Name, TempString);

			std::string OutputString = TempString.ToString();
			TempString.ResetNum();

			return OutputString;
		};

}

void fname::init_fallback()
{
	offsets::name::is_using_append_string_over_to_string  = false;

	MemAddress Conv_NameToStringAddress = FindUnrealExecFunctionByString("Conv_NameToString");

	constexpr std::array<const char*, 3> PossibleSigs =
	{
		"89 44 ? ? 48 01 ? ? E8",
		"48 89 ? ? 48 8D ? ? ? E8",
		"48 89 ? ? ? 48 89 ? ? E8",
	};

	int i = 0;
	while (!append_string && i < PossibleSigs.size())
	{
		append_string = static_cast<void(*)(const void*, fstring&)>(Conv_NameToStringAddress.RelativePattern(PossibleSigs[i], 0x90, -1 /* auto */));

		i++;
	}

	offsets::name::append_name_to_string = append_string ? GetOffset(append_string) : 0x0;
}

std::string fname::to_string() const
{
	if (!Address)
		return "None";

	std::string OutputString = to_str(Address);

	size_t pos = OutputString.rfind('/');

	if (pos == std::string::npos)
		return OutputString;

	return OutputString.substr(pos + 1);
}

std::string fname::to_valid_string() const
{
	return MakeNameValid(to_string());
}

int32 fname::get_comp_idx() const
{
	return *reinterpret_cast<const int32*>(Address + offsets::fname::comp_idx);
}

int32 fname::get_number() const
{
	return !Settings::Internal::bUseUoutlineNumberName ? *reinterpret_cast<const int32*>(Address + offsets::fname::number) : 0x0;
}

bool fname::operator==(fname Other) const
{
	return get_comp_idx() == Other.get_comp_idx();
}

bool fname::operator!=(fname Other) const
{
	return get_comp_idx() != Other.get_comp_idx();
}

std::string fname::comp_idx_to_string(int CmpIdx)
{
	if (!Settings::Internal::bUseCasePreservingName)
	{
		struct FakeFName
		{
			int CompIdx;
			uint8 Pad[0x4];
		} Name(CmpIdx);

		return fname(&Name).to_string();
	}
	else
	{
		struct FakeFName
		{
			int CompIdx;
			uint8 Pad[0xC];
		} Name(CmpIdx);

		return fname(&Name).to_string();
	}
}

void* fname::debug_get_append_string()
{
	return (void*)(append_string);
}
