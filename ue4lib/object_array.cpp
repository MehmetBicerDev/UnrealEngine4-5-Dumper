#include "object_array.h"
#include "util.h"
#include "settings.h"

#include <fstream>

/* Scuffed stuff up here */
struct FChunkedFixedUObjectArray
{
	void** ObjectsAbove;
	uint8_t Pad_0[0x08];
	int32_t MaxElements;
	int32_t NumElements;
	int32_t MaxChunks;
	int32_t NumChunks;
	void** ObjectsBelow;

	inline int32 IsValid(int32& OutObjectsPtrOffset)
	{
		void** ObjectsAboveButDecrypted = (void**)object_array::decrypt_ptr(ObjectsAbove);
		void** ObjectsBelowButDecrypted = (void**)object_array::decrypt_ptr(ObjectsBelow);

		if (NumChunks > 0x14 || NumChunks < 0x1)
			return false;

		if (MaxChunks > 0x22F || MaxChunks < 0x9)
			return false;

		if (NumElements > MaxElements || NumChunks > MaxChunks)
			return false;

		if (((NumElements / 0x10000) + 1) != NumChunks || (MaxElements / 0x10000) != MaxChunks)
			return false;

		const bool bAreObjectsAboveValid = (ObjectsAboveButDecrypted && !IsBadReadPtr(ObjectsAboveButDecrypted));
		const bool bAreObjectsBewlowValid = (ObjectsBelowButDecrypted && !IsBadReadPtr(ObjectsBelowButDecrypted));

		if (!bAreObjectsAboveValid && !bAreObjectsBewlowValid)
			return false;

		for (int i = 0; i < NumChunks; i++)
		{
#pragma warning(disable:6011)
			const bool bIsCurrentIndexValidAbove = bAreObjectsAboveValid ? !IsBadReadPtr(ObjectsAboveButDecrypted[i]) : false;
			const bool bIsCurrentIndexValidBelow = bAreObjectsBewlowValid ? !IsBadReadPtr(ObjectsBelowButDecrypted[i]) : false;
#pragma pop

			if (!bIsCurrentIndexValidAbove && !bIsCurrentIndexValidBelow)
				return false;
		}

		OutObjectsPtrOffset = 0x00;

		if (!bAreObjectsAboveValid && bAreObjectsBewlowValid)
			OutObjectsPtrOffset = 0x20;

		return true;
	}
};

struct FFixedUObjectArray
{
	struct FUObjectItem
	{
		void* Object;
		uint8_t Pad[0x10];
	};

	FUObjectItem* Objects;
	int32_t Max;
	int32_t Num;

	inline bool IsValid()
	{
		FUObjectItem* ObjectsButDecrypted = (FUObjectItem*)object_array::decrypt_ptr(Objects);

		if (Num > Max)
			return false;

		if (Max > 0x400000)
			return false;

		if (Num < 0x1000)
			return false;

		if (IsBadReadPtr(ObjectsButDecrypted))
			return false;

		if (IsBadReadPtr(ObjectsButDecrypted[5].Object))
			return false;

		if (*(int32_t*)(uintptr_t(ObjectsButDecrypted[5].Object) + 0xC) != 5)
			return false;

		return true;
	}
};


uint8_t* object_array::objects = nullptr;
uint32_t object_array::num_elements_per_chunk = 0x10000;
uint32_t object_array::size_of_fuobject_item = 0x18;
uint32_t object_array::fuobject_initial_item_offset = 0x0;
std::string object_array::decryption_lambda_str = "";



void object_array::initialize_fuobject_item(uint8_t* first_item_ptr)
{
	for (int i = 0x0; i < 0x10; i += 4)
	{
		if (!IsBadReadPtr(*reinterpret_cast<uint8_t**>(first_item_ptr + i)))
		{
			fuobject_initial_item_offset = i;
			break;//
		}
	}

	for (int i = fuobject_initial_item_offset + 0x8; i <= 0x38; i += 4)
	{
		void* SecondObject = *reinterpret_cast<uint8_t**>(first_item_ptr + i);
		void* ThirdObject = *reinterpret_cast<uint8_t**>(first_item_ptr + (i * 2) - fuobject_initial_item_offset);
		if (!IsBadReadPtr(SecondObject) && !IsBadReadPtr(*(void**)SecondObject) && !IsBadReadPtr(ThirdObject) && !IsBadReadPtr(*(void**)ThirdObject))
		{
			size_of_fuobject_item = i - fuobject_initial_item_offset;
			break;
		}
	}

	offsets::obj_array::fuobject_item_initial_offset = fuobject_initial_item_offset;
	offsets::obj_array::fuobject_item_size = size_of_fuobject_item;
}

void object_array::initialize_chunk_size(uint8_t* ChunksPtr)
{

	printf("Chunks Ptr %llx \n", ChunksPtr);
	int IndexOffset = 0x0;
	uint8* ObjAtIdx374 = (uint8*)by_index(ChunksPtr, 0x374, size_of_fuobject_item, fuobject_initial_item_offset, 0x10000);
	uint8* ObjAtIdx106 = (uint8*)by_index(ChunksPtr, 0x106, size_of_fuobject_item, fuobject_initial_item_offset, 0x10000);
	printf("ObjAtIdx374 Ptr %llx \n", ObjAtIdx374);
	printf("ObjAtIdx106 Ptr %llx \n", ObjAtIdx106);

	for (int i = 0x8; i < 0x20; i++)
	{
		if (*reinterpret_cast<int32*>(ObjAtIdx374 + i) == 0x374 && *reinterpret_cast<int32*>(ObjAtIdx106 + i) == 0x106)
		{
			IndexOffset = i;
			break;
		}
	}

	int IndexToCheck = 0x10400;
	while (object_array::num() > IndexToCheck)
	{
		if (void* Obj = by_index(ChunksPtr, IndexToCheck, size_of_fuobject_item, fuobject_initial_item_offset, 0x10000))
		{
			const bool bIsTrue = *reinterpret_cast<int32*>((uint8*)Obj + IndexOffset) != IndexToCheck;
			num_elements_per_chunk = bIsTrue ? 0x10400 : 0x10000;
			break;
		}
		IndexToCheck += 0x10400;
	}


	offsets::obj_array::chunk_size = num_elements_per_chunk;
}

void object_array::init_decryption(uint8_t* (*decryption_function)(void* ObjPtr), const char* decryption_lambda_as_str)
{
	decrypt_ptr = decryption_function;
	decryption_lambda_str = decryption_lambda_as_str;
}

void object_array::init(bool bScanAllMemory)
{
	if (!bScanAllMemory)
		std::cout << "\nDumper-7 by me, you & him\n\n\n";

	uintptr_t ImageBase = GetImageBase();
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)(ImageBase);
	PIMAGE_NT_HEADERS NtHeader = (PIMAGE_NT_HEADERS)(ImageBase + DosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER Sections = IMAGE_FIRST_SECTION(NtHeader);

	uint8_t* SearchBase = (uint8_t*)ImageBase;
	DWORD SearchRange = NtHeader->OptionalHeader.SizeOfImage;

	if (!bScanAllMemory)
	{
		for (int i = 0; i < NtHeader->FileHeader.NumberOfSections; i++)
		{
			IMAGE_SECTION_HEADER& CurrentSection = Sections[i];

			if (std::string((char*)CurrentSection.Name) == ".data")
			{
				SearchBase = (uint8_t*)(CurrentSection.VirtualAddress + ImageBase);
				SearchRange = CurrentSection.Misc.VirtualSize;
			}
		}
	}

	if (!bScanAllMemory)
		std::cout << "Searching for GObjects...\n\n";


	for (int i = 0; i < SearchRange; i += 0x4)
	{

		auto FixedArray = reinterpret_cast<FFixedUObjectArray*>(SearchBase + i);
		auto ChunkedArray = reinterpret_cast<FChunkedFixedUObjectArray*>(SearchBase + i);

		if (FixedArray->IsValid())
		{
			object_array::objects = reinterpret_cast<uint8_t*>(SearchBase + i);
			offsets::fuobject_array::num = 0xC;
			num_elements_per_chunk = -1;

			offsets::obj_array::objects = uintptr_t(SearchBase + i) - ImageBase;

			std::cout << "Found FFixedUObjectArray GObjects at offset 0x" << std::hex << offsets::obj_array::objects << std::dec << "\n\n";

			by_index = [](void* ObjectsArray, int32 Index, uint32 FUObjectItemSize, uint32 FUObjectItemOffset, uint32 PerChunk) -> void*
				{
					if (Index < 0 || Index > num())
						return nullptr;

					uint8_t* ChunkPtr = decrypt_ptr(*reinterpret_cast<uint8_t**>(ObjectsArray));

					return *reinterpret_cast<void**>(ChunkPtr + FUObjectItemOffset + (Index * FUObjectItemSize));
				};

			uint8_t* ChunksPtr = decrypt_ptr(*reinterpret_cast<uint8_t**>(object_array::objects + offsets::fuobject_array::ptr));

			object_array::initialize_fuobject_item(*reinterpret_cast<uint8_t**>(ChunksPtr));

			return;
		}
		else if (ChunkedArray->IsValid(offsets::fuobject_array::ptr))
		{
			object_array::objects = reinterpret_cast<uint8_t*>(SearchBase + i);
			num_elements_per_chunk = 0x10000;
			size_of_fuobject_item = 0x18;
			offsets::fuobject_array::num = 0x14;
			fuobject_initial_item_offset = 0x0;

			offsets::obj_array::objects = uintptr_t(SearchBase + i) - ImageBase;

			std::cout << "Found FChunkedFixedUObjectArray GObjects at offset 0x" << std::hex << offsets::obj_array::objects << std::dec << "\n\n";

			by_index = [](void* ObjectsArray, int32 Index, uint32 FUObjectItemSize, uint32 FUObjectItemOffset, uint32 PerChunk) -> void*
				{
					if (Index < 0 || Index > num())
						return nullptr;

					const int32 ChunkIndex = Index / PerChunk;
					const int32 InChunkIdx = Index % PerChunk;

					uint8_t* ChunkPtr = decrypt_ptr(*reinterpret_cast<uint8_t**>(ObjectsArray));

					uint8_t* Chunk = reinterpret_cast<uint8_t**>(ChunkPtr)[ChunkIndex];
					uint8_t* ItemPtr = reinterpret_cast<uint8_t*>(Chunk) + (InChunkIdx * FUObjectItemSize);

					return *reinterpret_cast<void**>(ItemPtr + FUObjectItemOffset);
				};

			uint8_t* ChunksPtr = decrypt_ptr(*reinterpret_cast<uint8_t**>(object_array::objects + offsets::fuobject_array::ptr));

			printf("Chunks ptr %llx \n", ChunksPtr);
			printf(" ptr %llx \n", offsets::fuobject_array::ptr);

			object_array::initialize_fuobject_item(*reinterpret_cast<uint8_t**>(ChunksPtr));

			object_array::initialize_chunk_size(object_array::objects + offsets::fuobject_array::ptr);

			return;
		}
	}

	if (!bScanAllMemory)
	{
		object_array::init(true);
		return;
	}

	if (!bScanAllMemory)
		std::cout << "\nGObjects couldn't be found!\n\n\n";
}

void object_array::init(int32_t objects_offset, int32_t elements_per_chunk, bool is_chunked)
{
	objects = reinterpret_cast<uint8_t*>(GetImageBase() + objects_offset);

	offsets::obj_array::objects = objects_offset;

	std::cout << "GObjects: 0x" << (void*)objects_offset << "\n" << std::endl;

	if (!is_chunked)
	{
		offsets::fuobject_array::num = 0xC;

		by_index = [](void* ObjectsArray, int32 Index, uint32 FUObjectItemSize, uint32 FUObjectItemOffset, uint32 PerChunk) -> void*
			{
				if (Index < 0 || Index > num())
					return nullptr;

				uint8_t* ItemPtr = *reinterpret_cast<uint8_t**>(ObjectsArray) + (Index * FUObjectItemSize);

				return *reinterpret_cast<void**>(ItemPtr + FUObjectItemOffset);
			};

		uint8_t* ChunksPtr = decrypt_ptr(*reinterpret_cast<uint8_t**>(objects_offset));

		object_array::initialize_fuobject_item(*reinterpret_cast<uint8_t**>(ChunksPtr));
	}
	else
	{
		offsets::fuobject_array::num = 0x14;

		by_index = [](void* ObjectsArray, int32 Index, uint32 FUObjectItemSize, uint32 FUObjectItemOffset, uint32 PerChunk) -> void*
			{
				if (Index < 0 || Index > num())
					return nullptr;

				const int32 ChunkIndex = Index / PerChunk;
				const int32 InChunkIdx = Index % PerChunk;

				uint8_t* Chunk = (*reinterpret_cast<uint8_t***>(ObjectsArray))[ChunkIndex];
				uint8_t* ItemPtr = reinterpret_cast<uint8_t*>(Chunk) + (InChunkIdx * FUObjectItemSize);

				return *reinterpret_cast<void**>(ItemPtr + FUObjectItemOffset);
			};

		uint8_t* ChunksPtr = decrypt_ptr(*reinterpret_cast<uint8_t**>(objects_offset));

		object_array::initialize_fuobject_item(*reinterpret_cast<uint8_t**>(ChunksPtr));
	}

	num_elements_per_chunk = elements_per_chunk;
	offsets::obj_array::chunk_size = elements_per_chunk;
}

void object_array::dump_objects(const std::filesystem::path& path)
{
	std::ofstream DumpStream(path / "GObjects-Dump.txt");

	DumpStream << "Object dump by Dumper-7\n\n";
	DumpStream << (!Settings::Generator::GameVersion.empty() && !Settings::Generator::GameName.empty() ? (Settings::Generator::GameVersion + '-' + Settings::Generator::GameName) + "\n\n" : "");
	DumpStream << "Count: " << num() << "\n\n\n";

	for (auto Object : object_array())
	{
		DumpStream << std::format("[{:08X}] {{{}}} {}\n", Object.get_index(), Object.get_address(), Object.get_full_name());
	}

	DumpStream.close();
}

int32_t object_array::num()
{
	return *reinterpret_cast<int32_t*>(objects + offsets::fuobject_array::num);
}

uclass object_array::find_class(std::string FullName)
{
	return find_objects<uclass>(FullName, EClassCastFlags::Class);
}

uclass object_array::find_class_fast(std::string Name)
{
	return find_object_fast<uclass>(Name, EClassCastFlags::Class);
}

object_array::objects_iterator object_array::begin()
{
	return objects_iterator(*this);
}

object_array::objects_iterator object_array::end()
{
	return objects_iterator(*this, num());
}

object_array::objects_iterator::objects_iterator(object_array& Array, int32_t StartIndex) : 
	iterated_array(Array), current_index(StartIndex), current_object(object_array::get_by_index(StartIndex))

{
}

uobject object_array::objects_iterator::operator*()
{
	return current_object;
}

object_array::objects_iterator& object_array::objects_iterator::operator++()
{
	current_object = object_array::get_by_index(++current_index);

	while (!current_object && current_index < (object_array::num() - 1))
	{
		current_object = object_array::get_by_index(++current_index);
	}

	if (!current_object && current_index == (object_array::num() - 1)) [[unlikely]]
		current_index++;

		return *this;
}

bool object_array::objects_iterator::operator!=(const objects_iterator& Other)
{
	return current_index != Other.current_index;
}

int32_t object_array::objects_iterator::get_index() const
{
	return current_index;
}
