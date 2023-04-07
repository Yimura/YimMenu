#include "locals_service.hpp"
#include "pointers.hpp"
#include "core/data/all_script_names.hpp"

namespace big{

    bool locals_service::is_script_thread_running(GtaThread* thread){
        if(thread)
        {
            return thread->m_context.m_state == rage::eThreadState::running || thread->m_context.m_state == rage::eThreadState::idle;
        }
        return false;
    }

    bool locals_service::does_script_exist(std::string script_name){
        for(auto s : all_script_names)
            if(script_name == s) return true;
        return false;
    }

	GtaThread* locals_service::find_script_thread (Hash hash)
	{
		for (auto thread : *g_pointers->m_script_threads)
		{
			if (thread
				&& thread->m_context.m_thread_id
				&& thread->m_handler
				&& thread->m_script_hash == hash)
			{
				return thread;
			}
		}

		return nullptr;
	}

    bool locals_service::load()
	{
		std::string path = std::getenv("appdata");
		path += this->file_location;

		std::ifstream file(path);

		if (!file.is_open())
			return false;

		try
		{
			nlohmann::json j;
			file >> j;
			this->m_locals.clear();
			for(const auto& l : j.items()){
				if(!l.key().empty()){
					local new_local{"", "", 0, 0, 0, 0};
					new_local.m_base_address = j[l.key()]["base_address"];
					new_local.m_script_thread_name = j[l.key()]["script_thread_name"];
					new_local.m_freeze = j[l.key()]["freeze"];
					new_local.m_name = j[l.key()]["name"];
					new_local.m_value = j[l.key()]["value"];
					if(!j[l.key()]["offsets"].is_null()){
						for(const auto& offset : j[l.key()]["offsets"].items()){
							if(!offset.key().empty()){
								local_offset new_offset{0,0};
								new_offset.m_offset = j[l.key()]["offsets"][offset.key()]["offset"];
								if(!j[l.key()]["offsets"][offset.key()]["size"].is_null())
									new_offset.m_size = j[l.key()]["offsets"][offset.key()]["size"];
								new_local.m_offsets.push_back(new_offset);
							}
						}
					}
					this->m_locals.push_back(new_local);
				}
			}
		}
		catch (const std::exception&)
		{
			LOG(WARNING) << "Failure to parse locals.json, aborting...";

			return false;
		}

		return true;
	}

	void locals_service::save()
	{
        std::map<std::string, local> locals_with_names;
        for (auto& l : m_locals){
            locals_with_names.insert(std::pair<std::string, local>(
                l.m_name.empty() ?  std::string(l.m_script_thread_name + "_" + std::to_string(l.m_base_address)) : l.m_name, l));
        }

		nlohmann::json j;
        for(auto& l : locals_with_names){
            j[l.first]["script_thread_name"] = l.second.m_script_thread_name;
            j[l.first]["base_address"] = l.second.m_base_address;
            j[l.first]["freeze"] = l.second.m_freeze;
            j[l.first]["name"] = l.second.m_name;
            j[l.first]["value"] = l.second.m_value;

            for (int i = 0; i < l.second.m_offsets.size(); i++)
            {
                j[l.first]["offsets"][std::to_string(i)]["offset"] = l.second.m_offsets[i].m_offset;
                if(l.second.m_offsets[i].m_size > 0) j[l.first]["offsets"][std::to_string(i)]["size"] = l.second.m_offsets[i].m_size;
            };
        }

		std::string path = std::getenv("appdata");
		path += this->file_location;
		std::ofstream file(path, std::ios::out | std::ios::trunc);

		try
		{
			file << j.dump(4);

			file.close();
		}
		catch (const std::exception&)
		{
			LOG(WARNING) << "Failed to write to locals.json";
		}
	}

};