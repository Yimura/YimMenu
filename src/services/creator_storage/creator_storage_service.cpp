#include "creator_storage_service.hpp"

#include "gta/joaat.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "script.hpp"
#include "script_function.hpp"
#include "script/tlsContext.hpp"

namespace big
{
	std::vector<std::string> creator_storage_service::list_files()
	{
		std::vector<std::string> file_paths;

		const auto file_path = check_jobs_folder();
		for (const auto& directory_entry : std::filesystem::directory_iterator(file_path.get_path()))
			if (directory_entry.path().extension() == ".json")
				file_paths.push_back(directory_entry.path().filename().generic_string());

		return file_paths;
	}

	std::filesystem::path creator_storage_service::create_file(std::string file)
	{
		return check_jobs_folder().get_file(file).get_path();
	}

	void creator_storage_service::save_file(std::string_view filename)
	{
		std::ofstream file(check_jobs_folder().get_file(filename).get_path());

		while (!SCRIPT::HAS_SCRIPT_WITH_NAME_HASH_LOADED("fm_race_creator"_J))
		{
			SCRIPT::REQUEST_SCRIPT_WITH_NAME_HASH("fm_race_creator"_J);
			script::get_current()->yield();
		}

		char* storage = new char[0x50000];
		scr_functions::save_to_datafile.call<void>((uint64_t)storage);
		delete[] storage;

		SCRIPT::SET_SCRIPT_WITH_NAME_HASH_AS_NO_LONGER_NEEDED("fm_race_creator"_J);

		auto buffer = g_pointers->m_gta.m_save_json_data(g_pointers->m_gta.m_main_file_object, nullptr, "to save it to a file I guess?");

		if (!buffer)
		{
			g_notification_service.push_error("CREATOR_STORAGE_SAVE_JOB"_T.data(),
			    "CREATOR_STORAGE_CANNOT_OBTAIN_JSON"_T.data());
			return;
		}

		file << buffer;
		file.close();

		rage::tlsContext::get()->m_allocator->Free(buffer);
	}

	void creator_storage_service::load_file(std::string_view file_name)
	{
		const auto file = check_jobs_folder().get_file(file_name);

		std::ifstream file_stream(file.get_path());
		std::string buffer(std::istreambuf_iterator<char>{file_stream}, {});

		if (DATAFILE::DATAFILE_GET_FILE_DICT(0) != nullptr)
			DATAFILE::DATAFILE_DELETE(0);

		sCloudFile* cloud_file = nullptr;
		g_pointers->m_gta.m_load_cloud_file(&cloud_file, buffer.data(), buffer.length(), "to load it from a file I guess?");
		g_pointers->m_gta.m_set_as_active_cloud_file(g_pointers->m_gta.m_main_file_object, &cloud_file);

		while (!SCRIPT::HAS_SCRIPT_WITH_NAME_HASH_LOADED("fm_race_creator"_J))
		{
			SCRIPT::REQUEST_SCRIPT_WITH_NAME_HASH("fm_race_creator"_J);
			script::get_current()->yield();
		}

		int load_stage   = 0; // Will be incremented at the end of each case in the function
		int ugc_language = NETWORK::UGC_GET_CONTENT_LANGUAGE(0);
		scr_functions::load_from_datafile.call<bool>(&load_stage, ugc_language, false);

		SCRIPT::SET_SCRIPT_WITH_NAME_HASH_AS_NO_LONGER_NEEDED("fm_race_creator"_J);
		file_stream.close();
	}

	big::folder creator_storage_service::check_jobs_folder()
	{
		const auto folder = g_file_manager.get_project_folder("./jobs");

		return folder;
	}
}
