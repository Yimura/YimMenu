#include "wren_manager.hpp"
#include "wren.hpp"
#include "wren_Vector3.hpp"
#include "wren_natives.hpp"
#include "script.hpp"
#include "fiber_pool.hpp"

namespace big
{
	static void wren_write(WrenVM* vm, const char* text)
	{
		if (text && *text)
		{
			size_t i = 0;
			while (text[i])
			{
				if (!isspace(text[i]))
				{
					LOG(INFO) << "[wren] " << text;
					return;
				}

				i++;
			}
		}
	}

	static void wren_error(WrenVM* vm, WrenErrorType errorType,
		const char* module, const int line,
		const char* msg)
	{
		switch (errorType)
		{
		case WREN_ERROR_COMPILE:
		{
			LOG(INFO) << "[wren] [" << module << " line " << line << "] [Error] " << msg;
		} break;
		case WREN_ERROR_STACK_TRACE:
		{
			LOG(INFO) << "[wren] [" << module << " line " << line << "] in " << msg;
		} break;
		case WREN_ERROR_RUNTIME:
		{
			LOG(INFO) << "[wren] [Runtime Error] " << msg;
		} break;
		}
	}

	static WrenForeignClassMethods wren_bind_foreign_class(
		WrenVM* vm, const char* module, const char* class_name)
	{
		WrenForeignClassMethods methods;

		if (strcmp(class_name, "Vector3") == 0)
		{
			methods.allocate = wren_Vector3::allocate;
			methods.finalize = NULL;
		}
		else
		{
			// Unknown class.
			methods.allocate = NULL;
			methods.finalize = NULL;
		}

		return methods;
	}

	static void wren_ScriptUtil_yield(WrenVM* vm)
	{
		script::get_current()->yield();
	}

	static WrenForeignMethodFn wren_bind_foreign_method(
		WrenVM* vm,
		const char* module,
		const char* class_name,
		bool is_static,
		const char* signature)
	{
		if (const auto it = wren_natives_handlers.find(signature); it != wren_natives_handlers.end())
		{
			return it->second;
		}

		if (strcmp(class_name, "ScriptUtil") == 0 && strcmp(signature, "yield()") == 0)
		{
			return wren_ScriptUtil_yield;
		}

		if (strcmp(class_name, "Vector3") == 0)
		{
			if (strcmp(signature, "x") == 0)
			{
				return wren_Vector3::get_x;
			}

			if (strcmp(signature, "y") == 0)
			{
				return wren_Vector3::get_y;
			}

			if (strcmp(signature, "z") == 0)
			{
				return wren_Vector3::get_z;
			}

			if (strcmp(signature, "x=(_)") == 0)
			{
				return wren_Vector3::set_x;
			}

			if (strcmp(signature, "y=(_)") == 0)
			{
				return wren_Vector3::set_y;
			}

			if (strcmp(signature, "z=(_)") == 0)
			{
				return wren_Vector3::set_z;
			}
		}

		return nullptr;
	}

	static void wren_load_module_complete(WrenVM* vm, const char* module, WrenLoadModuleResult result)
	{
		if (result.source)
		{
			delete[] result.source;
		}
	}

	static WrenLoadModuleResult wren_load_module(WrenVM* vm, const char* name)
	{
		WrenLoadModuleResult result = { 0 };

		g_wren_manager->for_each_wren_script_file([&](const auto& module_name, const auto& file_path, const auto& dir_entry)
		{
			if (strcmp(module_name.c_str(), name) == 0)
			{
				std::ifstream file_path_ifstream(file_path);
				std::stringstream buffer;
				buffer << file_path_ifstream.rdbuf();
				const auto script = buffer.str();

				char* heap_source_text = new char[script.size()];
				memcpy(heap_source_text, script.data(), script.size());

				result.source = heap_source_text;
				result.onComplete = wren_load_module_complete;

				return;
			}
		});

		return result;
	}

	void wren_manager::for_each_wren_script_file(std::function<void(const std::string& module_name, const std::filesystem::path& file_path, const std::filesystem::directory_entry& dir_entry)> cb)
	{
		for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(m_scripts_wren_folder))
		{
			if (dir_entry.is_regular_file())
			{
				const auto& file_path = dir_entry.path();
				if (file_path.extension() == ".wren" && file_path.has_stem())
				{
					const auto module = file_path.stem().u8string();

					cb(module, file_path, dir_entry);
				}
			}
		}
	}

	wren_manager::wren_manager() :
		m_scripts_folder(g_file_manager->get_project_folder("./scripts")),
		m_scripts_wren_folder(g_file_manager->get_project_folder("./scripts/wren").get_path())
	{
		wrenInitConfiguration(&m_config);
		m_config.writeFn = &wren_write;
		m_config.errorFn = &wren_error;
		m_config.bindForeignClassFn = &wren_bind_foreign_class;
		m_config.bindForeignMethodFn = &wren_bind_foreign_method;
		m_config.loadModuleFn = wren_load_module;

		m_vm = wrenNewVM(&m_config);

		g_fiber_pool->queue_job([this]
		{
			reload_scripts();
		});

		g_wren_manager = this;
	}

	wren_manager::~wren_manager()
	{
		// we call this there manually instead of letting the unique ptrs dctor trigger automatically
		// freeing the VM should be the last thing we do. (see https://wren.io/embedding/ for the why)
		remove_all_scripts();

		wrenFreeVM(m_vm);

		g_wren_manager = nullptr;
	}

	void wren_manager::remove_all_scripts()
	{
		m_wren_scripts.clear();
	}

	void wren_manager::reload_scripts()
	{
		struct script_to_load
		{
			std::string module_name;
			std::filesystem::path file_path;
			std::filesystem::file_time_type disk_last_write_time;
		};

		std::vector<script_to_load> scripts_to_load;

		for_each_wren_script_file([this, &scripts_to_load](const auto& module_name, const auto& file_path, const auto& dir_entry)
		{
			const std::filesystem::file_time_type& disk_last_write_time = dir_entry.last_write_time();

			if (const auto& it = m_wren_scripts.find(module_name); it != m_wren_scripts.end())
			{
				const auto& existing_script = it->second;
				const std::filesystem::file_time_type& existing_last_write_time = existing_script->last_write_time();
				bool script_changed = disk_last_write_time > existing_last_write_time;
				if (script_changed)
				{
					scripts_to_load.push_back({ module_name, file_path, disk_last_write_time });
				}
			}
			else
			{
				scripts_to_load.push_back({ module_name, file_path, disk_last_write_time });
			}
		});

		// no script got changed and no new scripts
		// aka nothing to reload / load
		if (!scripts_to_load.size())
		{
			return;
		}

		// if we already have some loaded scripts, we need to restart the VM
		if (m_wren_scripts.size())
		{
			remove_all_scripts();

			wrenFreeVM(m_vm);

			m_vm = wrenNewVM(&m_config);
		}

		for (const auto& script_to_load : scripts_to_load)
		{
			compile_script(script_to_load.module_name, script_to_load.file_path, script_to_load.disk_last_write_time);
		}
	}

	void wren_manager::compile_script(const std::string& module_name, const std::filesystem::path& file_path, const std::filesystem::file_time_type& disk_last_write_time)
	{
		std::ifstream file_path_ifstream(file_path);
		std::stringstream buffer;
		buffer << file_path_ifstream.rdbuf();
		const auto script = buffer.str();

		WrenInterpretResult result = wrenInterpret(m_vm, module_name.c_str(), script.c_str());

		switch (result)
		{
		case WREN_RESULT_COMPILE_ERROR:
			LOG(INFO) << "Compile error for " << file_path;

			break;
		case WREN_RESULT_RUNTIME_ERROR:
			LOG(INFO) << "Runtime error for " << file_path;

			break;
		case WREN_RESULT_SUCCESS:
		{
			LOG(INFO) << "Successfully executed " << file_path;

			bool has_script_class_declaration = false;
			bool has_tick_function = false;
			std::istringstream ss(script);
			std::string line;
			while (std::getline(ss, line))
			{
				if (line.find("class") != std::string::npos &&
					line.find("Script") != std::string::npos &&
					line.find("{") != std::string::npos)
				{
					has_script_class_declaration = true;
				}
				else if (has_script_class_declaration &&
					line.find("static") != std::string::npos &&
					line.find("tick") != std::string::npos &&
					line.find("{") != std::string::npos)
				{
					// make sure if there is any comment, its after the actual method declaration
					has_tick_function = line.find("//") > line.find("{") && line.find("/*") > line.find("{");

					break;
				}
			}

			m_wren_scripts[module_name] = std::make_unique<wren_script>(m_vm, module_name, disk_last_write_time, has_tick_function);

			break;
		}
		default:
			break;
		}
	}

	void wren_manager::tick_all_scripts()
	{
		while (g_running)
		{
			for (auto& [wren_script_module_name, wren_script] : g_wren_manager->m_wren_scripts)
			{
				wren_script->tick();
			}

			script::get_current()->yield();
		}
	}
}
