#include "settings.hpp"

#include "thread_pool.hpp"

namespace big
{
	void menu_settings::init(const file& save_file)
	{
		m_save_file = save_file;
		load();
	}

	void menu_settings::attempt_save()
	{
		const nlohmann::json j = *this;

		if (deep_compare(m_options, j, true))
			save();
	}

	bool menu_settings::load()
	{
		m_default_options = *this;

		std::ifstream file(m_save_file.get_path());

		if (!m_save_file.exists())
		{
			write_default_config();

			file.open(m_save_file.get_path());
		}

		try
		{
			file >> m_options;

			file.close();
		}
		catch (const std::exception&)
		{
			file.close();

			LOG(WARNING) << "Detected corrupt settings, writing default config...";

			write_default_config();

			return load();
		}

		const bool should_save = deep_compare(m_options, m_default_options);
		try
		{
			from_json(m_options, *this);
		}
		catch (const std::exception& e)
		{
			file.close();

			LOG(WARNING) << "Detected incompatible settings, writing default config: " << e.what();

			write_default_config();

			return load();
		}

		if (should_save)
		{
			LOG(INFO) << "Updating settings.";
			save();
		}

		return true;
	}

	bool menu_settings::write_default_config()
	{
		std::ofstream file(m_save_file.get_path(), std::ios::out | std::ios::trunc);
		file << m_default_options.dump(4);
		file.close();

		return true;
	}

	bool menu_settings::deep_compare(nlohmann::json& current_settings, const nlohmann::json& default_settings, bool compare_value)
	{
		bool should_save = false;

		for (auto& e : default_settings.items())
		{
			const std::string& key = e.key();

			if (current_settings.count(key) == 0 || (compare_value && current_settings[key] != e.value()))
			{
				current_settings[key] = e.value();

				should_save = true;
			}
			else if (current_settings[key].is_object() && e.value().is_object())
			{
				if (deep_compare(current_settings[key], e.value(), compare_value))
					should_save = true;
			}
			else if (!current_settings[key].is_object() && e.value().is_object())
			{
				current_settings[key] = e.value();

				should_save = true;
			}
			else if (current_settings[key].size() < e.value().size())
			{
				current_settings[key] = e.value();

				should_save = true;
			}
		}

		return should_save;
	}

	bool menu_settings::save()
	{
		std::ofstream file(m_save_file.get_path(), std::ios::out | std::ios::trunc);
		nlohmann::json j = *this;
		file << j.dump(4);
		file.close();

		return true;
	}
}