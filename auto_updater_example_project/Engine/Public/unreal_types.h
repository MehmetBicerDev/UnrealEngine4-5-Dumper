#pragma once

#include <array>
#include <string>
#include <iostream>
#include <Windows.h>
#include "Enums.h"
#include "util.h"
#include "Offsets.h"

extern std::string MakeNameValid(std::string&& Name);

template<typename ValueType, typename KeyType>
class tpair
{
public:
	ValueType First;
	KeyType Second;
};

template<class T>
class tarray
{
	friend class FString;

protected:
	T* Data;
	int32 NumElements;
	int32 MaxElements;

public:

	inline tarray()
		: Data(nullptr), NumElements(0), MaxElements(0)
	{
	}

	inline tarray(int32 Num)
		: NumElements(0), MaxElements(Num), Data((T*)malloc(sizeof(T)* Num))
	{
	}

	inline T& operator[](uint32 Index)
	{
		return Data[Index];
	}
	inline const T& operator[](uint32 Index) const
	{
		return Data[Index];
	}

	inline int32 Num() const
	{
		return NumElements;
	}

	inline int32 Max() const
	{
		return MaxElements;
	}

	inline int32 GetSlack() const
	{
		return Max() - Num();
	}

	inline bool IsEmpty() const
	{
		return Num() <= 0;
	}

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	inline bool IsValidIndex(int32 Index) const
	{
		return Data && Index >= 0 && Index < NumElements;
	}

	inline void ResetNum()
	{
		NumElements = 0;
	}

	inline void AddIfSpaceAvailable(const T& Element)
	{
		if (GetSlack() <= 0x0)
			return;

		Data[NumElements] = Element;
		NumElements++;
	}

	inline const void* GetDataPtr() const
	{
		return Data;
	}

protected:
	inline void FreeArray()
	{
		NumElements = 0;
		MaxElements = 0;
		if (Data) free(Data);
		Data = nullptr;
	}
};

class fstring : public tarray<wchar_t>
{
public:
	using tarray::tarray;

	inline fstring(const wchar_t* WChar)
	{
		MaxElements = NumElements = *WChar ? std::wcslen(WChar) + 1 : 0;

		if (NumElements)
		{
			Data = const_cast<wchar_t*>(WChar);
		}
	}

	inline fstring operator=(const wchar_t*&& Other)
	{
		return fstring(Other);
	}

	inline std::wstring ToWString() const
	{
		if (IsValid())
		{
			return Data;
		}

		return L"";
	}

	inline std::string ToString() const
	{
		if (IsValid())
		{
			std::wstring WData(Data);
			return std::string(WData.begin(), WData.end());
		}

		return "";
	}
};

class ffreablestring : public fstring
{
public:
	using fstring::fstring;

	~ffreablestring()
	{
		/* If we're using FName::ToString the allocation comes from the engine and we can not free it. Just leak those 2048 bytes. */
		if (offsets::name::is_using_append_string_over_to_string	)
			FreeArray();
	}
};


class fname
{
private:
	inline static void(*append_string)(const void*, fstring&) = nullptr;

	inline static std::string(*to_str)(const void* Name) = nullptr;

private:
	const uint8* Address;

public:
	fname() = default;

	fname(const void* Ptr);

public:
	static void init(bool bForceGNames = false);
	static void init_fallback();

	static void init(int32 AppendStringOffset, bool bIsToString = false);

public:
	inline const void* get_address() const { return Address; }

	std::string to_string() const;
	std::string to_valid_string() const;

	int32 get_comp_idx() const;
	int32 get_number() const;

	bool operator==(fname Other) const;

	bool operator!=(fname Other) const;

	static std::string comp_idx_to_string(int CmpIdx);

	static void* debug_get_append_string();
};
