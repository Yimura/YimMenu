#include "player_command.hpp"

#include "fiber_pool.hpp"

namespace big
{
	player_all_component::player_all_component(player_command* parent, const std::string& name, const std::string& label, const std::string& description, std::optional<uint8_t> num_args) :
	    command(name + "all", label, description, num_args),
	    m_parent(parent)
	{
	}

	void player_all_component::execute(const command_arguments& args, const std::shared_ptr<command_context> ctx)
	{
		g_fiber_pool->queue_job([this, args, ctx] {
			for (uint32_t i = 0; i < 32; ++i)
			{
				if (auto player = g_player_service->get_by_id(i); player && player->is_valid())
				{
					m_parent->execute(player, args, ctx);
				}
			}
		});
	}

	std::optional<command_arguments> player_all_component::parse_args(const std::vector<std::string>& args, const std::shared_ptr<command_context> ctx)
	{
		return m_parent->parse_args_p(args, ctx);
	}

	player_command::player_command(const std::string& name, const std::string& label, const std::string& description, std::optional<uint8_t> num_args, bool make_all_version) :
	    command(name, label, description, num_args.has_value() ? std::optional{num_args.value() + 1} : std::nullopt)
	{
		if (make_all_version)
			m_all_component = std::make_unique<player_all_component>(this, name, label, description, num_args);

		g_player_commands[rage::joaat(name)] = this;
	}

	void player_command::execute(const command_arguments& args, const std::shared_ptr<command_context> ctx)
	{
		g_fiber_pool->queue_job([this, args, ctx] {
			command_arguments new_args(m_num_args.value(), args);

			if (g_player_service->get_self()->id() == args.get<uint8_t>(0))
			{
				execute(g_player_service->get_self(), new_args, ctx);
				return;
			}

			for (auto& plyr : g_player_service->players())
			{
				if (plyr.second->id() == args.get<uint8_t>(0))
				{
					execute(plyr.second, new_args, ctx);
					return;
				}
			}

			ctx->report_error(std::format("Tried to execute command {}, but a player with index {} was not found", m_name, args.get<int>(0)));
		});
	}

	std::optional<command_arguments> player_command::parse_args(const std::vector<std::string>& args, const std::shared_ptr<command_context> ctx)
	{
		std::vector<std::string> new_args;
		command_arguments result(m_num_args.value());

		auto proxy_result = get_argument_proxy_value(args[0]);

		if (proxy_result.has_value())
		{
			result.push(proxy_result.value());
		}
		else
		{
			int plyr_id = -1;

			for (auto& plyr : g_player_service->players())
			{
				if (stricmp(plyr.second->get_name(), args[0].c_str()) == 0)
				{
					plyr_id = plyr.second->id();
					break;
				}
			}

			if (ctx->get_access_level() != CommandAccessLevel::ADMIN && (get_access_level() == CommandAccessLevel::TOXIC || get_access_level() == CommandAccessLevel::AGGRESSIVE) && plyr_id == self::id)
			{
				ctx->report_error("Permission denied, cannot call toxic commands on me");
				return std::nullopt;
			}

			if (plyr_id == -1)
			{
				ctx->report_error(std::format("Cannot find player with name {} in command {}", args[0], m_name));
				return std::nullopt;
			}

			result.push(plyr_id);
		}

		new_args.reserve(args.size() - 1);
		std::copy(++args.begin(), args.end(), new_args.begin());
		// for (int i = 1; i < args.size(); i++)
		// 	new_args.push_back(args[i]);

		auto res = parse_args_p(new_args, ctx);
		// no value indicates a failure
		if (!res.has_value())
			return std::nullopt;

		const auto alt_args = res.value();
		for (auto i = 0u; i < alt_args.size(); ++i)
			result.push(alt_args.get(i));

		return result;
	}

	void player_command::call(player_ptr player, const command_arguments& args, const std::shared_ptr<command_context> ctx)
	{
		if (m_num_args.has_value() && args.size() != (m_num_args.value() - 1))
		{
			ctx->report_error(std::format("Command {} called with the wrong number of arguments. Expected {}, got {}",
			    m_name,
			    m_num_args.value() - 1,
			    args.size()));
			return;
		}

		g_fiber_pool->queue_job([this, player, args, ctx] {
			if (player->is_valid())
				execute(player, args, ctx);
		});
	}
}