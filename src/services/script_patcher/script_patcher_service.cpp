#include "script_patcher_service.hpp"

#include "script_data.hpp"
#include "script_patch.hpp"

#include <script/scrProgram.hpp>

namespace big
{
	script_patcher_service::script_patcher_service()
	{
		g_script_patcher_service = this;
	}

	script_patcher_service::~script_patcher_service()
	{
		m_script_data.clear();
		m_script_patches.clear();
		g_script_patcher_service = nullptr;
	}

	script_data* script_patcher_service::get_data_for_script(rage::joaat_t script)
	{
		for (auto& p : m_script_data)
		{
			if (p.first == script)
			{
				return p.second.get();
			}
		}

		return nullptr;
	}

	bool script_patcher_service::does_script_have_patches(rage::joaat_t script)
	{
		for (auto& p : m_script_patches)
		{
			if (p.get_script() == script)
				return true;
		}

		return false;
	}

	void script_patcher_service::create_data_for_script(rage::scrProgram* program)
	{
		auto pages = new uint8_t*[program->get_num_code_pages()];

		for (auto i = 0u; i < program->get_num_code_pages(); i++)
		{
			pages[i] = new uint8_t[program->get_code_page_size(i)];
			std::memcpy(pages[i], program->get_code_page(i), program->get_code_page_size(i));
		}

		m_script_data.emplace(program->m_name_hash, std::make_unique<script_data>(program->m_code_size, pages, program->get_num_code_pages()));
	}

	void script_patcher_service::update_all_patches_for_script(rage::joaat_t script)
	{
		auto data = get_data_for_script(script);

		for (auto& p : m_script_patches)
			if (p.get_script() == script)
				p.update(data);
	}

	void script_patcher_service::add_patch(script_patch&& patch)
	{
		m_script_patches.push_back(std::move(patch));
	}

	void script_patcher_service::remove_patch(std::string_view patch_name)
	{
		for (auto it = m_script_patches.begin(); it != m_script_patches.end();)
		{
			if (it->get_name() == patch_name)
			{
				it = m_script_patches.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void script_patcher_service::on_script_load(rage::scrProgram* program)
	{
		if (!does_script_have_patches(program->m_name_hash))
			return;

		if (get_data_for_script(program->m_name_hash) == nullptr)
			create_data_for_script(program);

		update_all_patches_for_script(program->m_name_hash);
	}

	uint8_t** script_patcher_service::get_script_bytecode(rage::joaat_t script)
	{
		if (auto data = get_data_for_script(script))
			return data->m_bytecode;

		return nullptr;
	}

	void script_patcher_service::update()
	{
		for (auto& p : m_script_patches)
		{
			auto data = get_data_for_script(p.get_script());
			if (data)
			{
				p.update(data);
			}
		}
	}
}