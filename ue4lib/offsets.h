#pragma once
#include "ue4lib_includes.h"

namespace offsets {
	void init();
	void init_engine();
	void init_settings();
	void init_weakobjectptr_settings();
	void init_largeworld_coordinate_settings();

	namespace process_event
	{
		inline int32_t pe_index;
		inline int32_t pe_offset;

		void init_pe();
		void init_pe(int32_t index);
	}

	namespace world
	{
		inline int32_t world_offset = 0x0;

		void init_world();
	}

	namespace obj_array
	{
		inline int32_t objects = 0;
		inline int32_t chunk_size = 0;
		inline int32_t fuobject_item_size = 0;
		inline int32_t fuobject_item_initial_offset = 0;
	}

	namespace name 
	{
		inline bool is_using_append_string_over_to_string = true;
		inline int32_t append_name_to_string;
		inline int32_t fname_size;
	}

	namespace properties
	{
		inline int32_t property_size;
	}

	namespace text
	{
		inline int32_t text_dat_offset = 0x0;
		inline int32_t in_text_data_string_offset = 0x0;
		inline int32_t text_size = 0x0;

		void init_text_offsets();
	}

	namespace fuobject_array
	{
		inline int32_t ptr = 0;
		inline int32_t num = 0;
	}

	namespace name_array
	{
		inline int32_t chunks_start;
		inline int32_t max_chunk_index;
		inline int32_t num_elements;
		inline int32_t byte_cursor;
		inline int32_t GNames = 0x0;
		inline int32_t fnamepool_block_offset_bits = 0x0;
		inline int32_t fname_entry_Stride = 0x0;
	}

	namespace ffield
	{
		inline int32_t vft = 0x00;
		inline int32_t klass = 0x08;
		inline int32_t owner = 0x10;
		inline int32_t next = 0x20;
		inline int32_t name = 0x28;
		inline int32_t flags = 0x30;
	}

	namespace ffieldclass
	{
		inline int32_t name = 0x00;
		inline int32_t id = 0x08;
		inline int32_t cast_flags = 0x10;
		inline int32_t class_flags = 0x18;
		inline int32_t super_class = 0x20;
	}

	namespace fname
	{
		inline int32_t comp_idx = 0x0;
		inline int32_t number = 0x04;
	}

	namespace fname_entry
	{
		namespace name_array
		{
			inline int32_t string_offset;
			inline int32_t index_offset;
		}

		namespace namepool
		{
			inline int32_t header_offset;
			inline int32_t string_offset;
		}
	}

	namespace uobjecto
	{
		inline int32_t vft;
		inline int32_t flags;
		inline int32_t index;
		inline int32_t klass;
		inline int32_t name;
		inline int32_t outer;
	}

	namespace ufieldo
	{
		inline int32_t next;
	}

	namespace uenumo
	{
		inline int32_t names;
	}

	namespace ustructo
	{
		inline int32_t super_struct;
		inline int32_t children;
		inline int32_t child_properties;
		inline int32_t size;
		inline int32_t min_alignment;
	}

	namespace ufunctiono
	{
		inline int32_t function_flags;
		inline int32_t exec_function;
	}

	namespace uclasso
	{
		inline int32_t cast_flags;
		inline int32_t class_default_object;
	}

	namespace fproperty
	{
		inline int32_t array_dim;
		inline int32_t elements_size;
		inline int32_t property_flags;
		inline int32_t offset_internal;
	}

	namespace byte_property
	{
		inline int32_t senum;
	}

	namespace bool_property
	{
		struct ubool_propety_base
		{
			uint8_t field_size;
			uint8_t byte_offset;
			uint8_t byte_mask;
			uint8_t field_mask;
		};

		inline int32_t base;
	}

	namespace object_property
	{
		inline int32_t property_class;
	}

	namespace class_property
	{
		inline int32_t meta_class;
	}

	namespace struct_property
	{
		inline int32_t sstruct;
	}

	namespace array_property
	{
		inline int32_t inner;
	}

	namespace delegate_property
	{
		inline int32_t signature_function;
	}

	namespace map_property
	{
		struct umap_property_base
		{
			void* key_property;
			void* value_property;
		};

		inline int32_t base;
	}

	namespace set_property
	{
		inline int32_t element_prop;
	}

	namespace enum_property
	{
		struct uenum_property_base
		{
			void* underlaying_property;
			class uenum* senum;
		};

		inline int32_t base;
	}

	namespace field_path_property
	{
		inline int32_t field_class;
	}

	namespace optional_property
	{
		inline int32_t value_property;
	};

	namespace ulevelo
	{
		inline int32_t actors;
	}
}