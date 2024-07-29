#pragma once
#include <unreal_objects.h>
#include "unreal_struct.h"





namespace dumper {
	namespace class_manager {
		void process_class(unreal_package* owner, uobject& object, bool is_class);
	}
}