#include "backend/context/default_command_context.hpp"
#include "pointers.hpp"
#include "util/string_operations.hpp"
#include "views/view.hpp"

namespace big
{
	//TODO Allow for optional arguments??
	static std::vector<std::string> current_suggestion_list;
	static std::string command_buffer;
	static std::string auto_fill_suggestion;
	static std::string selected_suggestion;
	bool suggestion_is_history = false;
	static int cursor_pos      = 0;

	struct argument
	{
		std::string name;
		int index;
		int start_index;
		int end_index;
		bool is_argument = true; // If the argument is the command itself, this will be false
	};

	struct command_scope
	{
		command* cmd;
		std::string raw;
		std::string name; // If the command is not found, this will be the incomplete command
		int name_start_index;
		int name_end_index;
		int index;
		int start_index;
		int end_index;
		int argument_count;
		std::vector<argument> arguments;

		argument* get_argument(int cursor_pos)
		{
			auto found = std::find_if(arguments.begin(), arguments.end(), [&](const argument& arg) {
				return cursor_pos >= arg.start_index && cursor_pos <= arg.end_index;
			});

			if (found != arguments.end())
				return &*found;

			return nullptr;
		}
	};

	static void clean_buffer(std::string& buffer)
	{
		std::string new_buffer;
		bool last_char_was_space = false;

		for (size_t i = 0; i < buffer.size(); ++i)
		{
			if (buffer[i] == ' ')
			{
				// Skip consecutive spaces
				if (!last_char_was_space)
				{
					new_buffer += ' ';
					last_char_was_space = true;
				}
			}
			else if (buffer[i] == ';')
			{
				new_buffer += ';';
				// Skip spaces after a semicolon
				while (i + 1 < buffer.size() && buffer[i + 1] == ' ')
				{
					++i;
				}
				last_char_was_space = false;
			}
			else
			{
				new_buffer += buffer[i];
				last_char_was_space = false;
			}
		}

		// Remove leading and trailing spaces (optional, if needed)
		size_t start = new_buffer.find_first_not_of(' ');
		size_t end   = new_buffer.find_last_not_of(' ');
		if (start == std::string::npos || end == std::string::npos)
		{
			buffer.clear(); // No non-space characters found
		}
		else
		{
			buffer = new_buffer.substr(start, end - start + 1);
		}
	}

	class serialized_buffer
	{
		std::string buffer;
		int total_length;
		int command_count;
		std::vector<command_scope> command_scopes;

	public:
		serialized_buffer(std::string buffer) :
		    buffer(buffer),
		    total_length(0),
		    command_count(0)
		{
			if (buffer.empty())
				return;

			clean_buffer(buffer);
			parse_buffer();
		}

		void parse_buffer()
		{
			auto separate_commands = string::operations::split(buffer, ';');

			command_count = separate_commands.size();
			total_length  = 0;

			for (int i = 0; i < command_count; i++)
			{
				auto words = string::operations::split(separate_commands[i], ' ');
				auto cmd   = command::get(rage::joaat(words.front()));

				command_scope scope;
				scope.cmd            = cmd;
				scope.name           = words.front();
				scope.index          = i;
				scope.start_index    = total_length;
				scope.raw            = separate_commands[i];
				scope.argument_count = words.size() - 1;

				size_t buffer_pos = total_length;

				for (int j = 0; j < words.size(); j++)
				{
					size_t word_start = buffer.find(words[j], buffer_pos);

					argument arg;
					arg.name        = words[j];
					arg.index       = j;
					arg.is_argument = j > 0;
					arg.start_index = word_start;
					arg.end_index   = word_start + words[j].size();
					scope.arguments.push_back(arg);

					buffer_pos = word_start + words[j].size();
					if (j < words.size() - 1)
					{
						buffer_pos++; // Move past the space
					}
				}

				scope.end_index = buffer_pos;
				total_length    = buffer_pos + 1; // Move past the semicolon or end of command

				command_scopes.push_back(scope);
			}
		}

		std::string deserialize()
		{
			if (command_count == 0)
				return std::string();

			std::string deserialized_buffer;
			for (auto& command : command_scopes)
			{
				for (auto& argument : command.arguments)
				{
					deserialized_buffer += argument.name;

					if (argument.name != command.arguments.back().name)
						deserialized_buffer += ' ';
				}

				if (command.raw != command_scopes.back().raw)
					deserialized_buffer += ';';
			}

			return deserialized_buffer;
		}

		command_scope* get_command_scope(int cursor_pos)
		{
			auto found = std::find_if(command_scopes.begin(), command_scopes.end(), [&](const command_scope& scope) {
				return cursor_pos >= scope.start_index && cursor_pos <= scope.end_index;
			});

			if (found != command_scopes.end())
				return &*found;

			return nullptr;
		}

		bool is_current_index_argument(int cursor_pos)
		{
			auto* scope = get_command_scope(cursor_pos);

			if (!scope)
				return false;

			auto* argument = scope->get_argument(cursor_pos);

			if (!argument)
				return false;

			return argument->is_argument;
		}

		int get_argument_index_from_char_index(int cursor_pos)
		{
			auto* scope = get_command_scope(cursor_pos);

			if (!scope)
				return -1;

			auto* argument = scope->get_argument(cursor_pos);

			if (!argument)
				return -1;

			return argument->index;
		}

		command* get_command_of_index(int cursor_pos)
		{
			auto* scope = get_command_scope(cursor_pos);

			if (!scope)
				return nullptr;

			return scope->cmd;
		}

		void update_argument_of_scope(int index, int arg, std::string new_argument)
		{
			auto* scope = get_command_scope(index);

			if (!scope)
				return;

			auto* argument = scope->get_argument(index);

			if (!argument)
				return;

			auto original_arg_textlen = argument->name.length();
			auto new_arg_textlen      = new_argument.length();
			auto len_diff             = new_arg_textlen - original_arg_textlen; // Can be negative
			argument->name            = new_argument;

			for (int i = scope->index; i < command_count; i++)
			{
				auto& current_scope = command_scopes[i];

				if (current_scope.index == scope->index)
				{
					current_scope.end_index += len_diff;
				}

				for (auto& current_argument : current_scope.arguments)
				{
					if (current_argument.index == argument->index)
					{
						current_argument.end_index += len_diff;
					}
				}
			}
		}
	};

	static serialized_buffer s_buffer(command_buffer);

	bool does_string_exist_in_list(const std::string& command, std::vector<std::string> list)
	{
		auto found = std::find(list.begin(), list.end(), command);
		return found != list.end();
	}

	std::vector<std::string> deque_to_vector(std::deque<std::string> deque)
	{
		std::vector<std::string> vector;
		for (auto& element : deque)
		{
			vector.push_back(element);
		}
		return vector;
	}

	static void add_to_last_used_commands(const std::string& command)
	{
		if (does_string_exist_in_list(command, deque_to_vector(g.cmd.command_history)))
		{
			return;
		}

		if (g.cmd.command_history.size() >= 10)
		{
			g.cmd.command_history.pop_back();
		}

		g.cmd.command_history.push_front(command);
	}

	std::string auto_fill_command(std::string current_buffer)
	{
		if (command::get(rage::joaat(current_buffer)) != nullptr)
			return current_buffer;

		for (auto [key, cmd] : g_commands)
		{
			if (cmd && cmd->get(key) && &cmd->get_name())
			{
				if (cmd->get_name().find(current_buffer) != std::string::npos)
					return cmd->get_name();
			}
		}

		return std::string();
	}

	std::vector<std::string> suggestion_list_filtered(std::vector<std::string> suggestions, std::string filter)
	{
		std::vector<std::string> suggestions_filtered;
		std::string filter_lowercase = filter;
		string::operations::to_lower(filter_lowercase);

		auto current_scope = s_buffer.get_command_scope(cursor_pos);

		if (!current_scope)
			return suggestions;

		auto argument = current_scope->get_argument(cursor_pos);

		if (!argument)
			return suggestions;

		for (auto suggestion : suggestions)
		{
			std::string suggestion_lowercase = suggestion;
			string::operations::to_lower(suggestion_lowercase);

			if (suggestion_lowercase.find(filter_lowercase) != std::string::npos || does_string_exist_in_list(argument->name, suggestions) /*Need this to maintain suggestion list while navigating it*/)
				suggestions_filtered.push_back(suggestion);
		}

		return suggestions_filtered;
	}

	void get_appropriate_suggestion(std::string current_buffer, std::string& suggestion_)
	{
		auto serbuffer       = serialized_buffer(current_buffer);
		auto current_command = serbuffer.get_command_of_index(cursor_pos);
		auto scope           = serbuffer.get_command_scope(cursor_pos);
		auto argument_index  = serbuffer.get_argument_index_from_char_index(cursor_pos);

		if (!scope)
		{
			return;
		}

		if (argument_index == -1)
		{
			return;
		}

		if (!scope->get_argument(cursor_pos)->is_argument)
		{
			suggestion_ = auto_fill_command(scope->name);
			return;
		}

		if (!current_command)
			return;

		auto suggestions = current_command->get_argument_suggestions(argument_index);
		auto argument    = scope->get_argument(cursor_pos);

		if (suggestions == std::nullopt)
			return;

		for (auto suggestion : suggestion_list_filtered(suggestions.value(), argument->name))
		{
			std::string guess_lowercase      = argument->name;
			std::string suggestion_lowercase = suggestion;
			string::operations::to_lower(suggestion_lowercase);
			string::operations::to_lower(guess_lowercase);

			if (suggestion_lowercase.find(guess_lowercase) != std::string::npos)
			{
				suggestion_ = suggestion;
				break;
			}
		}
	}

	void get_previous_from_list(std::vector<std::string>& list, std::string& current)
	{
		auto found = std::find_if(list.begin(), list.end(), [&](const std::string& cmd) {
			return cmd == current;
		});

		if (found == list.end())
		{
			if (list.size() > 0)
				current = list.back();

			return;
		}

		if (*found == list.front())
		{
			current = list.back();
			return;
		}

		if (found - 1 != list.end())
			current = *(found - 1);
	}

	void get_next_from_list(std::vector<std::string>& list, std::string& current)
	{
		auto found = std::find_if(list.begin(), list.end(), [&](const std::string& cmd) {
			return cmd == current;
		});

		if (found == list.end())
		{
			if (list.size() > 0)
				current = list.front();

			return;
		}

		if (*found == list.back())
		{
			current = list.front();
			return;
		}

		if (found + 1 != list.end())
			current = *(found + 1);
	}

	void update_current_argument_with_suggestion(ImGuiInputTextCallbackData* data, std::string suggestion)
	{
		if (!data)
			return;

		std::string new_text;
		auto sbuffer        = serialized_buffer(data->Buf);
		auto scope          = sbuffer.get_command_scope(data->CursorPos);
		auto argument_index = sbuffer.get_argument_index_from_char_index(data->CursorPos);

		if (!scope)
			return;

		if (argument_index == -1)
			return;

		auto argument = scope->get_argument(data->CursorPos);

		if (!argument)
			return;

		sbuffer.update_argument_of_scope(data->CursorPos, argument_index, suggestion);

		new_text = sbuffer.deserialize();

		data->DeleteChars(0, data->BufTextLen);
		data->InsertChars(0, new_text.c_str());
		data->CursorPos = argument->end_index;
	}

	bool buffer_needs_cleaning(const std::string& input)
	{
		for (size_t i = 0; i < input.size(); ++i)
		{
			if (input[i] == ' ')
			{
				if (i + 1 < input.size() && input[i + 1] == ' ')
				{
					return true; // Consecutive spaces
				}
			}
			else if (input[i] == ';')
			{
				if (i + 1 < input.size() && (input[i + 1] == ';' || input[i + 1] == ' '))
				{
					return true; // Consecutive semicolons or space after semicolon
				}
			}
		}
		return false;
	}

	static int input_callback(ImGuiInputTextCallbackData* data)
	{
		if (!data)
		{
			return 0;
		}
		
		if (cursor_pos != data->CursorPos)
		{
			selected_suggestion = std::string();
			cursor_pos          = data->CursorPos;

			if (buffer_needs_cleaning(data->Buf))
			{
				std::string cleaned_buffer = data->Buf;
				clean_buffer(cleaned_buffer);
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, cleaned_buffer.c_str());
			}
		}

		if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
		{
			// User has a suggestion selectable higlighted, this takes precedence
			if (!selected_suggestion.empty())
			{
				if (suggestion_is_history)
				{
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, selected_suggestion.c_str());
				}
				else
				{
					update_current_argument_with_suggestion(data, selected_suggestion);
				}

				selected_suggestion = std::string();
				return 0;
			}

			std::string auto_fill_suggestion;
			get_appropriate_suggestion(data->Buf, auto_fill_suggestion);

			if (auto_fill_suggestion != data->Buf)
			{
				update_current_argument_with_suggestion(data, auto_fill_suggestion);
			}
		}
		else if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
		{
			if (current_suggestion_list.empty())
				return 0;

			if (data->EventKey == ImGuiKey_UpArrow)
				get_previous_from_list(current_suggestion_list, selected_suggestion);
			else if (data->EventKey == ImGuiKey_DownArrow)
				get_next_from_list(current_suggestion_list, selected_suggestion);

			if (!selected_suggestion.empty() && !suggestion_is_history)
			{
				auto scope = s_buffer.get_command_scope(data->CursorPos);

				if (!scope)
					return 0;

				auto argument = scope->get_argument(data->CursorPos);

				if (!argument)
					return 0;

				data->SelectionStart = argument->start_index;
				data->SelectionEnd   = argument->end_index;
			}
		}
		
		return 0;
	}

	void view::cmd_executor()
	{
		if (!g.cmd_executor.enabled || g_pointers->m_gta.m_get_last_keyboard_state() == KeyboardState::ACTIVE)
			return;

		float screen_x = (float)*g_pointers->m_gta.m_resolution_x;
		float screen_y = (float)*g_pointers->m_gta.m_resolution_y;

		ImGui::SetNextWindowPos(ImVec2(screen_x * 0.25f, screen_y * 0.2f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.65f);
		ImGui::SetNextWindowSize({screen_x * 0.5f, -1});

		if (ImGui::Begin("cmd_executor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMouseInputs))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {10.f, 15.f});
			components::sub_title("CMD_EXECUTOR_TITLE"_T);

			// set focus by default on input box
			ImGui::SetKeyboardFocusHere(0);
			ImGui::SetNextItemWidth((screen_x * 0.5f) - 30.f);
			s_buffer = serialized_buffer(command_buffer); // Update serialized buffer every frame
			if (components::input_text_with_hint("", "CMD_EXECUTOR_TYPE_CMD"_T, command_buffer, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackAlways, nullptr, input_callback))
			{
				if (!s_buffer.get_command_of_index(cursor_pos))
					goto VIEW_END;

				if (command::process(command_buffer, std::make_shared<default_command_context>(), false))
				{
					g.cmd_executor.enabled = false;
					add_to_last_used_commands(command_buffer);
					command_buffer      = {};
					selected_suggestion = std::string();
				}
			}

			if (!command_buffer.empty())
			{
				get_appropriate_suggestion(command_buffer, auto_fill_suggestion);

				if (auto_fill_suggestion != command_buffer)
					ImGui::Text("Suggestion: %s", auto_fill_suggestion.data());
			}

			components::small_text("CMD_EXECUTOR_MULTIPLE_CMDS"_T);
			components::small_text("CMD_EXECUTOR_INSTRUCTIONS"_T);
			ImGui::Separator();
			ImGui::Spacing();

			if (suggestion_is_history)
				components::sub_title("CMD_HISTORY_LABEL"_T);

			if (current_suggestion_list.size() > 0)
			{
				for (auto suggestion : current_suggestion_list)
				{
					components::selectable(suggestion, suggestion == selected_suggestion);
				}
			}
			else
			{
				components::small_text("CMD_EXECUTOR_NO_SUGGESTIONS"_T);
			}

			// Show history if buffer is empty
			if (command_buffer.empty())
			{
				suggestion_is_history = true;

				if (!g.cmd.command_history.empty())
				{
					current_suggestion_list = deque_to_vector(g.cmd.command_history);
				}
			}
			// If buffer isn't empty, we rely on the serialized buffer to suggest arguments or commands
			else
			{
				suggestion_is_history = false;
				auto current_scope    = s_buffer.get_command_scope(cursor_pos);

				if (!current_scope)
					goto VIEW_END;

				auto argument = current_scope->get_argument(cursor_pos);

				if (!argument)
					goto VIEW_END;

				auto current_command = current_scope->cmd;

				if (argument->is_argument && current_command)
				{
					auto argument_suggestions = current_command->get_argument_suggestions(argument->index);
					if (argument_suggestions != std::nullopt)
					{
						auto filtered_suggestions = suggestion_list_filtered(argument_suggestions.value(), argument->name);
						if (filtered_suggestions.size() > 10)
						{
							current_suggestion_list =
							    std::vector<std::string>(filtered_suggestions.begin(), filtered_suggestions.begin() + 10);
						}
						else
						{
							current_suggestion_list = filtered_suggestions;
						}
					}
				}
				else
				{
					auto all_commands = g_commands;
					std::vector<std::string> command_names{};
					for (auto& [hash, cmd] : all_commands)
					{
						if (cmd && cmd->get_name().length() > 0)
							command_names.push_back(cmd->get_name());
					}

					auto filtered_commands = suggestion_list_filtered(command_names, argument->name);
					if (filtered_commands.size() > 10)
					{
						current_suggestion_list = std::vector<std::string>(filtered_commands.begin(), filtered_commands.begin() + 10);
					}
					else
					{
						current_suggestion_list = filtered_commands;
					}
				}
			}
		VIEW_END:
			ImGui::PopStyleVar();
		}

		ImGui::End();
	}

	bool_command g_cmd_executor("cmdexecutor", "CMD_EXECUTOR", "CMD_EXECUTOR_DESC", g.cmd_executor.enabled, false);
}