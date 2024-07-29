#pragma once
#include "unreal_types.h"

class fnameentry
{
private:
	friend class name_array;

private:
	static constexpr int32 NameWideMask = 0x1;

private:
	static inline int32 FNameEntryLengthShiftCount = 0x0;

	static inline std::string(*GetStr)(uint8* NameEntry) = nullptr;

private:
	uint8* Address;

public:
	fnameentry() = default;

	fnameentry(void* Ptr);

public:
	std::string get_string();
	void* get_address();

private:
	//Optional to avoid code duplication for FNamePool
	static void init(const uint8_t* FirstChunkPtr = nullptr, int64 NameEntryStringOffset = 0x0);
};

class name_array
{
private:
	static inline uint32 FNameBlockOffsetBits = 0x10;

private:
	static uint8* GNames;

	static inline int64 NameEntryStride = 0x0;

	static inline void* (*by_index)(void* NamesArray, int32 ComparisonIndex, int32 NamePoolBlockOffsetBits) = nullptr;

private:
	static bool initialize_name_array(uint8_t* NameArray);
	static bool initialize_name_pool(uint8_t* NamePool);

public:
	/* Should be changed later and combined */
	static bool try_find_name_array();
	static bool try_find_name_pool();

	static bool try_init(bool bIsTestOnly = false);

	/* Initializes the GNames offset, but doesn't call NameArray::InitializeNameArray() or NameArray::InitializedNamePool() */
	static bool set_gnames_without_commiting();

	static void post_init();

public:
	static int32 get_num_chunks();

	static int32 get_num_elements();
	static int32 get_byte_cursor();

	static fnameentry get_name_entry(const void* Name);
	static fnameentry get_name_entry(int32 Idx);
};
