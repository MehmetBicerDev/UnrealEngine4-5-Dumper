#include "name_array.h"
#include "settings.h"
/* DEBUG */
#include "object_array.h"

uint8* name_array::GNames = nullptr;

fnameentry::fnameentry(void* Ptr)
	: Address((uint8*)Ptr)
{
}

std::string fnameentry::get_string()
{
	if (!Address)
		return "";

	return GetStr(Address);
}

void* fnameentry::get_address()
{
	return Address;
}

void fnameentry::init(const uint8_t* FirstChunkPtr, int64 NameEntryStringOffset)
{
	if (Settings::Internal::bUseNamePool)
	{
		constexpr int64 NoneStrLen = 0x4;
		constexpr uint16 BytePropertyStrLen = 0xC;

		constexpr uint32 BytePropertyStartAsUint32 = 'etyB'; // "Byte" part of "ByteProperty"

		offsets::fname_entry::namepool::string_offset = NameEntryStringOffset;
		offsets::fname_entry::namepool::header_offset = NameEntryStringOffset == 6 ? 4 : 0;

		uint8* AssumedBytePropertyEntry = *reinterpret_cast<uint8* const*>(FirstChunkPtr) + NameEntryStringOffset + NoneStrLen;

		/* Check if there's pading after an FNameEntry. Check if there's up to 0x4 bytes padding. */
		for (int i = 0; i < 0x4; i++)
		{
			const uint32 FirstPartOfByteProperty = *reinterpret_cast<const uint32*>(AssumedBytePropertyEntry + NameEntryStringOffset);

			if (FirstPartOfByteProperty == BytePropertyStartAsUint32)
				break;

			AssumedBytePropertyEntry += 0x1;
		}

		uint16 BytePropertyHeader = *reinterpret_cast<const uint16*>(AssumedBytePropertyEntry + offsets::fname_entry::namepool::header_offset);

		/* Shifiting past the size of the header is not allowed, so limmit the shiftcount here */
		constexpr int32 MaxAllowedShiftCount = sizeof(BytePropertyHeader) * 0x8;

		while (BytePropertyHeader != BytePropertyStrLen && FNameEntryLengthShiftCount < MaxAllowedShiftCount)
		{
			FNameEntryLengthShiftCount++;
			BytePropertyHeader >>= 1;
		}

		if (FNameEntryLengthShiftCount == MaxAllowedShiftCount)
		{
			std::cout << "\Dumper-7: Error, couldn't get FNameEntryLengthShiftCount!\n" << std::endl;
			GetStr = [](uint8* NameEntry)->std::string { return "Invalid FNameEntryLengthShiftCount!"; };
			return;
		}

		GetStr = [](uint8* NameEntry) -> std::string
			{
				const uint16 HeaderWithoutNumber = *reinterpret_cast<uint16*>(NameEntry + offsets::fname_entry::namepool::header_offset);
				const int32 NameLen = HeaderWithoutNumber >> fnameentry::FNameEntryLengthShiftCount;

				if (NameLen == 0)
				{
					const int32 EntryIdOffset = offsets::fname_entry::namepool::string_offset + ((offsets::fname_entry::namepool::string_offset == 6) * 2);

					const int32 NextEntryIndex = *reinterpret_cast<int32*>(NameEntry + EntryIdOffset);
					const int32 Number = *reinterpret_cast<int32*>(NameEntry + EntryIdOffset + sizeof(int32));

					if (Number > 0)
						return name_array::get_name_entry(NextEntryIndex).get_string() + "_" + std::to_string(Number - 1);

					return name_array::get_name_entry(NextEntryIndex).get_string();
				}

				if (HeaderWithoutNumber & NameWideMask)
				{
					std::wstring WString(reinterpret_cast<const wchar_t*>(NameEntry + offsets::fname_entry::namepool::string_offset), NameLen);
					return std::string(WString.begin(), WString.end());
				}

				return std::string(reinterpret_cast<const char*>(NameEntry + offsets::fname_entry::namepool::string_offset), NameLen);
			};
	}
	else
	{
		uint8_t* FNameEntryNone = (uint8_t*)name_array::get_name_entry(0x0).get_address();
		uint8_t* FNameEntryIdxThree = (uint8_t*)name_array::get_name_entry(0x3).get_address();
		uint8_t* FNameEntryIdxEight = (uint8_t*)name_array::get_name_entry(0x8).get_address();

		for (int i = 0; i < 0x20; i++)
		{
			if (*reinterpret_cast<uint32*>(FNameEntryNone + i) == 'enoN') // None
			{
				offsets::fname_entry::name_array::string_offset = i;
				break;
			}
		}

		for (int i = 0; i < 0x20; i++)
		{
			// lowest bit is bIsWide mask, shift right by 1 to get the index
			if ((*reinterpret_cast<uint32*>(FNameEntryIdxThree + i) >> 1) == 0x3 &&
				(*reinterpret_cast<uint32*>(FNameEntryIdxEight + i) >> 1) == 0x8)
			{
				offsets::fname_entry::name_array::index_offset = i;
				break;
			}
		}

		GetStr = [](uint8* NameEntry) -> std::string
			{
				const int32 NameIdx = *reinterpret_cast<int32*>(NameEntry + offsets::fname_entry::name_array::index_offset);
				const void* NameString = reinterpret_cast<void*>(NameEntry + offsets::fname_entry::name_array::string_offset);

				if (NameIdx & NameWideMask)
				{
					std::wstring WString(reinterpret_cast<const wchar_t*>(NameString));
					return std::string(WString.begin(), WString.end());
				}

				return reinterpret_cast<const char*>(NameString);
			};
	}
}

bool name_array::initialize_name_array(uint8_t* NameArray)
{
	int32 ValidPtrCount = 0x0;
	int32 ZeroQWordCount = 0x0;

	int32 PerChunk = 0x0;

	if (!NameArray || IsBadReadPtr(NameArray))
		return false;

	for (int i = 0; i < 0x800; i += 0x8)
	{
		uint8_t* SomePtr = *reinterpret_cast<uint8_t**>(NameArray + i);

		if (SomePtr == 0)
		{
			ZeroQWordCount++;
		}
		else if (ZeroQWordCount == 0x0 && SomePtr != nullptr)
		{
			ValidPtrCount++;
		}
		else if (ZeroQWordCount > 0 && SomePtr != 0)
		{
			int32 NumElements = *reinterpret_cast<int32_t*>(NameArray + i);
			int32 NumChunks = *reinterpret_cast<int32_t*>(NameArray + i + 4);

			if (NumChunks == ValidPtrCount)
			{
				offsets::name_array::num_elements = i;
				offsets::name_array::max_chunk_index = i + 4;

				by_index = [](void* NamesArray, int32 ComparisonIndex, int32 NamePoolBlockOffsetBits) -> void*
					{
						const int32 ChunkIdx = ComparisonIndex / 0x4000;
						const int32 InChunk = ComparisonIndex % 0x4000;

						if (ComparisonIndex > name_array::get_num_elements())
							return nullptr;

						return reinterpret_cast<void***>(NamesArray)[ChunkIdx][InChunk];
					};

				return true;
			}
		}
	}

	return false;
}

bool name_array::initialize_name_pool(uint8_t* NamePool)
{
	offsets::name_array::max_chunk_index = 0x0;
	offsets::name_array::byte_cursor = 0x4;

	offsets::name_array::chunks_start = 0x10;

	bool bWasMaxChunkIndexFound = false;

	for (int i = 0x0; i < 0x20; i += 4)
	{
		const int32 PossibleMaxChunkIdx = *reinterpret_cast<int32*>(NamePool + i);

		if (PossibleMaxChunkIdx <= 0 || PossibleMaxChunkIdx > 0x10000)
			continue;

		int32 NotNullptrCount = 0x0;
		bool bFoundFirstPtr = false;

		for (int j = 0x0; j < 0x10000; j += 8)
		{
			const int32 ChunkOffset = i + 8 + j + (i % 8);

			if ((*reinterpret_cast<uint8_t**>(NamePool + ChunkOffset)) != nullptr)
			{
				NotNullptrCount++;

				if (!bFoundFirstPtr)
				{
					bFoundFirstPtr = true;
					offsets::name_array::chunks_start = i + 8 + j + (i % 8);
				}
			}
		}

		if (PossibleMaxChunkIdx == (NotNullptrCount - 1))
		{
			offsets::name_array::max_chunk_index = i;
			offsets::name_array::byte_cursor = i + 4;
			bWasMaxChunkIndexFound = true;
			break;
		}
	}

	if (!bWasMaxChunkIndexFound)
		return false;

	constexpr uint64 CoreUObjAsUint64 = 0x6A624F5565726F43; // little endian "jbOUeroC" ["/Script/CoreUObject"]
	constexpr uint32 NoneAsUint32 = 0x656E6F4E; // little endian "None"

	constexpr int64 CoreUObjectStringLength = sizeof("/S");

	uint8_t** ChunkPtr = reinterpret_cast<uint8_t**>(NamePool + offsets::name_array::chunks_start);

	// "/Script/CoreUObject"
	bool bFoundCoreUObjectString = false;
	int64 FNameEntryHeaderSize = 0x0;

	constexpr int32 LoopLimit = 0x1000;

	for (int i = 0; i < LoopLimit; i++)
	{
		if (*reinterpret_cast<uint32*>(*ChunkPtr + i) == NoneAsUint32 && FNameEntryHeaderSize == 0)
		{
			FNameEntryHeaderSize = i;
		}
		else if (*reinterpret_cast<uint64*>(*ChunkPtr + i) == CoreUObjAsUint64)
		{
			bFoundCoreUObjectString = true;
			break;
		}
	}

	if (!bFoundCoreUObjectString)
		return false;

	NameEntryStride = FNameEntryHeaderSize == 2 ? 2 : 4;
	offsets::name_array::fname_entry_Stride = NameEntryStride;

	by_index = [](void* NamesArray, int32 ComparisonIndex, int32 NamePoolBlockOffsetBits) -> void*
		{
			const int32 ChunkIdx = ComparisonIndex >> NamePoolBlockOffsetBits;
			const int32 InChunkOffset = (ComparisonIndex & ((1 << NamePoolBlockOffsetBits) - 1)) * NameEntryStride;

			const bool bIsBeyondLastChunk = ChunkIdx == name_array::get_num_chunks() && InChunkOffset > name_array::get_byte_cursor();

			if (ChunkIdx < 0 || ChunkIdx > get_num_chunks() || bIsBeyondLastChunk)
				return nullptr;

			uint8_t* ChunkPtr = reinterpret_cast<uint8_t*>(NamesArray) + 0x10;

			return reinterpret_cast<uint8_t**>(ChunkPtr)[ChunkIdx] + InChunkOffset;
		};

	Settings::Internal::bUseNamePool = true;
	fnameentry::init(reinterpret_cast<uint8*>(ChunkPtr), FNameEntryHeaderSize);

	return true;
}

//
/*
 * Finds a call to	GetNames, OR a reference to GNames directly, if the call has been inlined
 *
 * returns { GetNames/GNames, bIsGNamesDirectly };
*/
inline std::pair<uintptr_t, bool> find_fname_getnames_or_gnames(uintptr_t EnterCriticalSectionAddress, uintptr_t StartAddress)
{
	/* 2 bytes operation + 4 bytes relative offset */
	constexpr int32 ASMRelativeCallSizeBytes = 0x6;

	/* Range from "ByteProperty" which we want to search upwards for "GetNames" call */
	constexpr int32 GetNamesCallSearchRange = 0x150;

	/* Find a reference to the string "ByteProperty" in 'FName::StaticInit' */
	const uint8* BytePropertyStringAddress = static_cast<uint8*>(FindByStringInAllSections(L"ByteProperty", StartAddress));

	/* Important to prevent infinite-recursion */
	if (!BytePropertyStringAddress)
		return { 0x0, false };

	for (int i = 0; i < GetNamesCallSearchRange; i++)
	{
		/* Check upwards (yes negative indexing) for a relative call opcode */
		if (BytePropertyStringAddress[-i] != 0xFF)
			continue;

		uintptr_t CallTarget = ASMUtils::Resolve32BitSectionRelativeCall(reinterpret_cast<uintptr_t>(BytePropertyStringAddress - i));

		if (CallTarget != EnterCriticalSectionAddress)
			continue;

		uintptr_t InstructionAfterCall = reinterpret_cast<uintptr_t>(BytePropertyStringAddress - (i - ASMRelativeCallSizeBytes));

		/* Check if we're dealing with a 'call' opcode */
		if (*reinterpret_cast<const uint8*>(InstructionAfterCall) == 0xE8)
			return { ASMUtils::Resolve32BitRelativeCall(InstructionAfterCall), false };

		return { ASMUtils::Resolve32BitRelativeMove(InstructionAfterCall), true };
	}

	/* Continue and search for another reference to "ByteProperty", safe because we're checking if another string-ref was found*/
	return find_fname_getnames_or_gnames(EnterCriticalSectionAddress, reinterpret_cast<uintptr_t>(BytePropertyStringAddress));
};

bool name_array::try_find_name_array()
{
	/* Type of 'static TNameEntryArray& FName::GetNames()' */
	using GetNameType = void* (*)();

	/* Range from 'FName::GetNames' which we want to search down for 'mov register, GNames' */
	constexpr int32 GetNamesCallSearchRange = 0x100;

	void* EnterCriticalSectionAddress = GetImportAddress(nullptr, "kernel32.dll", "EnterCriticalSection");

	auto [Address, bIsGNamesDirectly] = find_fname_getnames_or_gnames(reinterpret_cast<uintptr_t>(EnterCriticalSectionAddress), GetImageBase());

	if (Address == 0x0)
		return false;

	if (bIsGNamesDirectly)
	{
		if (!IsInProcessRange(Address) || IsBadReadPtr(*reinterpret_cast<void**>(Address)))
			return false;

		offsets::name_array::GNames = GetOffset(reinterpret_cast<void*>(Address));
		return true;
	}

	/* Call GetNames to retreive the pointer to the allocation of the name-table, used for later comparison */
	void* Names = reinterpret_cast<GetNameType>(Address)();

	for (int i = 0; i < GetNamesCallSearchRange; i++)
	{
		/* Check upwards (yes negative indexing) for a relative call opcode */
		if (*reinterpret_cast<const uint16*>(Address + i) != 0x8B48)
			continue;

		uintptr_t MoveTarget = ASMUtils::Resolve32BitRelativeMove(Address + i);

		if (!IsInProcessRange(MoveTarget))
			continue;

		void* ValueOfMoveTargetAsPtr = *reinterpret_cast<void**>(MoveTarget);

		if (IsBadReadPtr(ValueOfMoveTargetAsPtr) || ValueOfMoveTargetAsPtr != Names)
			continue;

		offsets::name_array::GNames = GetOffset(reinterpret_cast<void*>(MoveTarget));
		return true;
	}

	return false;
}

bool name_array::try_find_name_pool()
{
	/* Number of bytes we want to search for an indirect call to InitializeSRWLock */
	constexpr int32 InitSRWLockSearchRange = 0x50;

	/* Number of bytes we want to search for lea instruction loading the string "ByteProperty" */
	constexpr int32 BytePropertySearchRange = 0x2A0;

	/* FNamePool::FNamePool contains a call to InitializeSRWLock or RtlInitializeSRWLock, we're going to check for that later */
	//uintptr_t InitSRWLockAddress = reinterpret_cast<uintptr_t>(GetImportAddress(nullptr, "kernel32.dll", "InitializeSRWLock"));
	uintptr_t InitSRWLockAddress = reinterpret_cast<uintptr_t>(GetAddressOfImportedFunctionFromAnyModule("kernel32.dll", "InitializeSRWLock"));
	uintptr_t RtlInitSRWLockAddress = reinterpret_cast<uintptr_t>(GetAddressOfImportedFunctionFromAnyModule("ntdll.dll", "RtlInitializeSRWLock"));

	/* Singleton instance of FNamePool, which is passed as a parameter to FNamePool::FNamePool */
	void* NamePoolIntance = nullptr;

	uintptr_t SigOccurrence = 0x0;;

	uintptr_t Counter = 0x0;

	while (!NamePoolIntance)
	{
		/* add 0x1 so we don't find the same occurence again and cause an infinite loop (20min. of debugging for that) */
		if (SigOccurrence > 0x0)
			SigOccurrence += 0x1;

		/* Find the next occurence of this signature to see if that may be a call to the FNamePool constructor */
		SigOccurrence = reinterpret_cast<uintptr_t>(FindPattern("48 8D 0D ? ? ? ? E8", 0x0, true, SigOccurrence));

		if (SigOccurrence == 0x0)
			break;

		constexpr int32 SizeOfMovInstructionBytes = 0x7;

		uintptr_t PossibleConstructorAddress = ASMUtils::Resolve32BitRelativeCall(SigOccurrence + SizeOfMovInstructionBytes);

		if (!IsInProcessRange(PossibleConstructorAddress))
			continue;

		for (int i = 0; i < InitSRWLockSearchRange; i++)
		{
			/* Check for a relative call with the opcodes FF 15 00 00 00 00 */
			if (*reinterpret_cast<uint16*>(PossibleConstructorAddress + i) != 0x15FF)
				continue;

			uintptr_t RelativeCallTarget = ASMUtils::Resolve32BitSectionRelativeCall(PossibleConstructorAddress + i);

			if (!IsInProcessRange(RelativeCallTarget))
				continue;

			uintptr_t ValueOfCallTarget = *reinterpret_cast<uintptr_t*>(RelativeCallTarget);

			if (ValueOfCallTarget != InitSRWLockAddress && ValueOfCallTarget != RtlInitSRWLockAddress)
				continue;

			/* Try to find the "ByteProperty" string, as it's always referenced in FNamePool::FNamePool, so we use it to verify that we got the right function */
			void* StringRef = FindByStringInAllSections(L"ByteProperty", PossibleConstructorAddress, BytePropertySearchRange);

			/* We couldn't find a wchar_t string L"ByteProperty", now see if we can find a char string "ByteProperty" */
			if (!StringRef)
				StringRef = FindByStringInAllSections("ByteProperty", PossibleConstructorAddress, BytePropertySearchRange);

			if (StringRef)
			{
				NamePoolIntance = reinterpret_cast<void*>(ASMUtils::Resolve32BitRelativeMove(SigOccurrence));
				break;
			}
		}
	}

	if (NamePoolIntance)
	{
		offsets::name_array::GNames = GetOffset(NamePoolIntance);
		return true;
	}

	return false;
}

bool name_array::try_init(bool bIsTestOnly)
{
	uintptr_t ImageBase = GetImageBase();

	uint8* GNamesAddress = nullptr;

	bool bFoundNameArray = false;
	bool bFoundnamePool = false;

	if (name_array::try_find_name_array())
	{
		GNamesAddress = *reinterpret_cast<uint8**>(ImageBase + offsets::name_array::GNames);// Derefernce
		Settings::Internal::bUseNamePool = false;
		bFoundNameArray = true;
	}
	else if (name_array::try_find_name_pool())
	{
		GNamesAddress = reinterpret_cast<uint8*>(ImageBase + offsets::name_array::GNames); // No derefernce
		Settings::Internal::bUseNamePool = true;
		bFoundnamePool = true;
	}

	if (!bFoundNameArray && !bFoundnamePool)
	{
		std::cout << "\n\nCould not find GNames!\n\n" << std::endl;
		return false;
	}

	if (bIsTestOnly)
		return false;

	if (bFoundNameArray && name_array::initialize_name_array(GNamesAddress))
	{
		GNames = GNamesAddress;
		Settings::Internal::bUseNamePool = false;
		fnameentry::init();
		return true;
	}
	else if (bFoundnamePool && name_array::initialize_name_pool(reinterpret_cast<uint8_t*>(GNamesAddress)))
	{
		GNames = GNamesAddress;
		Settings::Internal::bUseNamePool = true;
		/* FNameEntry::Init() was moved into NameArray::InitializeNamePool to avoid duplicated logic */
		return true;
	}



	return false;
}

bool name_array::set_gnames_without_commiting()
{
	/* GNames is already set */
	if (offsets::name_array::GNames != 0x0)
		return false;

	if (name_array::try_find_name_array())
	{
		Settings::Internal::bUseNamePool = false;
		return true;
	}
	else if (name_array::try_find_name_pool())
	{
		Settings::Internal::bUseNamePool = true;
		return true;
	}

	return false;
}

void name_array::post_init()
{
	if (GNames && Settings::Internal::bUseNamePool)
	{
		// Reverse-order iteration because newer objects are more likely to have a chunk-index equal to NumChunks - 1

		name_array::FNameBlockOffsetBits = 0xE;

		int i = object_array::num();
		while (i >= 0)
		{
			const int32 CurrentBlock = name_array::get_num_chunks();

			uobject Obj = object_array::get_by_index(i);

			if (!Obj)
			{
				i--;
				continue;
			}

			const int32 ObjNameChunkIdx = Obj.get_fname().get_comp_idx() >> name_array::FNameBlockOffsetBits;

			if (ObjNameChunkIdx == CurrentBlock)
				break;

			if (ObjNameChunkIdx > CurrentBlock)
			{
				name_array::FNameBlockOffsetBits++;
				i = object_array::num();
			}

			i--;
		}
		offsets::name_array::fnamepool_block_offset_bits = name_array::FNameBlockOffsetBits;

	}
}

int32 name_array::get_num_chunks()
{
	return *reinterpret_cast<int32*>(GNames + offsets::name_array::max_chunk_index);
}

int32 name_array::get_num_elements()
{
	return !Settings::Internal::bUseNamePool ? *reinterpret_cast<int32*>(GNames + offsets::name_array::num_elements) : 0;
}

int32 name_array::get_byte_cursor()
{
	return Settings::Internal::bUseNamePool ? *reinterpret_cast<int32*>(GNames + offsets::name_array::byte_cursor) : 0;
}

fnameentry name_array::get_name_entry(const void* Name)
{
	return by_index(GNames, fname(Name).get_comp_idx(), FNameBlockOffsetBits);
}

fnameentry name_array::get_name_entry(int32 Idx)
{
	return by_index(GNames, Idx, FNameBlockOffsetBits);
}

