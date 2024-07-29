#pragma once
#include "c_generator.h"



namespace dumper {
	class c_internal_autoupdater_generator : public c_generator
	{
	public:

		bool is_blocked_type(std::string type, std::string name);
		void create_engine_files(std::string path);
		void create_directories(std::string path);

		std::string dump_sdk(std::string package_name, unreal_struct* klass);
		// Inherited via c_generator
		void generate(std::string path, std::vector<unreal_package*> packages) override;

	};
}

static std::unique_ptr<dumper::c_internal_autoupdater_generator> internal_autoupdater_generator = std::make_unique<dumper::c_internal_autoupdater_generator>();