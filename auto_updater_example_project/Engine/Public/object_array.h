#pragma once
#include "ue4lib_includes.h"
#include "unreal_objects.h"
class object_array
{
private:
	friend struct FChunkedFixedUObjectArray;
	friend struct FFixedUObjectArray;
	friend class ObjectArrayValidator;

private:
	static uint8_t* objects;
	static uint32_t num_elements_per_chunk;
	static uint32_t size_of_fuobject_item;
	static uint32_t fuobject_initial_item_offset;
public:
	static std::string decryption_lambda_str;

private:
	static inline void* (*by_index)(void* ObjectsArray, int32_t Index, uint32_t FUObjectItemSize, uint32_t FUObjectItemOffset, uint32_t PerChunk) = nullptr;

	static inline uint8_t* (*decrypt_ptr)(void* ObjPtr) = [](void* Ptr) -> uint8_t* { return (uint8_t*)Ptr; };

private:
	static void initialize_fuobject_item(uint8_t* first_item_ptr);
	static void initialize_chunk_size(uint8_t* objects);

public:
	static void init_decryption(uint8_t* (*decryption_function)(void* ObjPtr), const char* decryption_lambda_as_str);

	static void init(bool scan_all_memory = false);

	static void init(int32_t objects_offset, int32_t num_elements_per_chunk, bool is_chunked);

	static void dump_objects(const std::filesystem::path& path);

	static int32_t num();

	template<typename utype = uobject>
	static utype get_by_index(int32_t Index);

	template<typename utype = uobject>
	static utype find_objects(std::string FullName, EClassCastFlags RequiredType = EClassCastFlags::None);

	template<typename utype = uobject>
	static utype find_object_fast(std::string Name, EClassCastFlags RequiredType = EClassCastFlags::None);

	template<typename utype = uobject>
	static utype find_object_fast_in_outer(std::string Name, std::string Outer);

	static uclass find_class(std::string FullName);

	static uclass find_class_fast(std::string Name);


	class objects_iterator
	{
		object_array& iterated_array;
		uobject current_object;
		int32_t current_index;

	public:
		objects_iterator(object_array& Array, int32_t StartIndex = 0);

		uobject operator*();
		objects_iterator& operator++();
		bool operator!=(const objects_iterator& Other);

		int32_t get_index() const;
	};

	objects_iterator begin();
	objects_iterator end();

	static inline void* DEBUGGetGObjects()
	{
		return objects;
	}
};


#define init_object_array_descryption(DecryptionLambda) object_array::init_decryption(DecryptionLambda, #DecryptionLambda)

template<typename utype>
inline utype object_array::get_by_index(int32_t Index)
{
	return utype(by_index(objects + offsets::fuobject_array::ptr, Index, size_of_fuobject_item, fuobject_initial_item_offset, num_elements_per_chunk));
}

template<typename utype>
inline utype object_array::find_objects(std::string FullName, EClassCastFlags RequiredType)
{
	for (uobject Object : object_array())
	{
		if (Object.isa(RequiredType) && Object.get_full_name() == FullName)
		{
			return Object.cast<utype>();
		}
	}

	return utype();
}

template<typename utype>
inline utype object_array::find_object_fast(std::string Name, EClassCastFlags RequiredType)
{
	auto ObjArray = object_array();

	for (uobject Object : ObjArray)
	{
		if (Object.isa(RequiredType) && Object.get_name() == Name)
		{
			return Object.cast<utype>();
		}
	}

	return utype();
}

template<typename utype>
inline utype object_array::find_object_fast_in_outer(std::string Name, std::string Outer)
{
	auto ObjArray = object_array();

	for (uobject Object : ObjArray)
	{
		if (Object.get_name() == Name && Object.get_outer().get_name() == Outer)
		{
			return Object.cast<utype>();
		}
	}

	return utype();
}
