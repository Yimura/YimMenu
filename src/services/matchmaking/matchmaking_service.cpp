#include "matchmaking_service.hpp"

#include "fiber_pool.hpp"
#include "hooking/hooking.hpp"
#include "script.hpp"

#include <network/Network.hpp>

namespace big
{
	matchmaking_service::matchmaking_service()
	{
		g_matchmaking_service = this;
	}

	matchmaking_service::~matchmaking_service()
	{
		g_matchmaking_service = nullptr;
	}

	void matchmaking_service::patch_matchmaking_attributes(MatchmakingAttributes* attributes)
	{
		if (g.spoofing.spoof_session_region_type)
			attributes->m_param_values[4] = g.spoofing.session_region_type;

		if (g.spoofing.spoof_session_language)
			attributes->m_param_values[3] = (uint32_t)g.spoofing.session_language;

		if (g.spoofing.spoof_session_player_count && g.spoofing.increase_player_limit)
			attributes->m_param_values[2] = std::min(29, g.spoofing.session_player_count);
		else if (g.spoofing.spoof_session_player_count)
			attributes->m_param_values[2] = g.spoofing.session_player_count;
		else if (g.spoofing.increase_player_limit)
			attributes->m_param_values[2] = std::min(29u, attributes->m_param_values[2]);

		// TODO: the logic is incorrect

		attributes->m_param_values[0] = -0x22F37A9E;

		if (g.spoofing.spoof_session_bad_sport_status == 1)
			attributes->m_param_values[0] |= (1 << 14); // Bad Sport

		if (g.spoofing.spoof_session_bad_sport_status == 2)
			attributes->m_param_values[0] &= ~(1 << 14); // Good Sport
	}

	bool matchmaking_service::matchmake(std::optional<int> constraint, std::optional<bool> enforce_player_limit)
	{
		for (auto& session : m_found_sessions)
		{
			session.is_valid = true;
		}

		NetworkGameFilterMatchmakingComponent component{};
		strcpy(component.m_filter_name, "Group");
		component.m_game_mode      = 0;
		component.m_num_parameters = 0;

		if (g.session_browser.region_filter_enabled)
		{
			component.SetParameter("MMATTR_REGION", 0, g.session_browser.region_filter);
		}

		if (constraint)
		{
			component.SetParameter("MMATTR_DISCRIMINATOR", 1, constraint.value());
		}

		rage::rlTaskStatus state{};
		static rage::rlSessionInfo result_sessions[MAX_SESSIONS_TO_FIND];

		m_active             = true;
		m_num_valid_sessions = 0;

		if (g_hooking->get_original<hooks::start_matchmaking_find_sessions>()(0, 1, &component, MAX_SESSIONS_TO_FIND, result_sessions, &m_num_sessions_found, &state))
		{
			while (state.status == 1)
				script::get_current()->yield();

			if (state.status == 3)
			{
				std::unordered_map<std::uint64_t, session*> stok_map = {};

				for (int i = 0; i < m_num_sessions_found; i++)
				{
					m_found_sessions[i].info = result_sessions[i];

					if (auto it = stok_map.find(m_found_sessions[i].info.m_session_token); it != stok_map.end())
					{
						if (g.session_browser.filter_multiplexed_sessions)
						{
							it->second->is_valid = false;
						}

						it->second->attributes.multiplex_count++;
						m_found_sessions[i].is_valid = false;
						continue;
					}

					if (enforce_player_limit.has_value() && enforce_player_limit.value()
					    && m_found_sessions[i].attributes.player_count >= 30)
						m_found_sessions[i].is_valid = false;

					if (g.session_browser.language_filter_enabled
					    && (eGameLanguage)m_found_sessions[i].attributes.language != g.session_browser.language_filter)
						m_found_sessions[i].is_valid = false;

					if (g.session_browser.player_count_filter_enabled
					    && (m_found_sessions[i].attributes.player_count < g.session_browser.player_count_filter_minimum
					        || m_found_sessions[i].attributes.player_count > g.session_browser.player_count_filter_maximum))
					{
						m_found_sessions[i].is_valid = false;
					}

					if (g.session_browser.pool_filter_enabled
					    && ((m_found_sessions[i].attributes.discriminator & (1 << 14)) == (1 << 14))
					        != (bool)g.session_browser.pool_filter)
						m_found_sessions[i].is_valid = false;

					stok_map.emplace(m_found_sessions[i].info.m_session_token, &m_found_sessions[i]);
				}

				if (g.session_browser.sort_method != 0)
				{
					std::qsort(m_found_sessions, m_num_sessions_found, sizeof(session), [](const void* a1, const void* a2) -> int {
						std::strong_ordering result;

						if (g.session_browser.sort_method == 1)
						{
							result = (((session*)(a1))->attributes.player_count <=> ((session*)(a2))->attributes.player_count);
						}

						if (result == 0)
							return 0;

						if (result > 0)
							return g.session_browser.sort_direction ? -1 : 1;

						if (result < 0)
							return g.session_browser.sort_direction ? 1 : -1;


						std::unreachable();
					});
				}

				m_active = false;
				return true;
			}
		}
		else
		{
			m_active = false;
			return false;
		}

		m_active = false;
		return false;
	}

	bool matchmaking_service::handle_advertise(int num_slots, int available_slots, rage::rlSessionInfo* info, MatchmakingAttributes* attributes, MatchmakingId* out_id, rage::rlTaskStatus* status)
	{
		patch_matchmaking_attributes(attributes);

		if (!g.spoofing.multiplex_session)
			return false;

		if (status->status)
			return true;

		status->status = 1; // set in progress

		// create the first advertisement
		g_fiber_pool->queue_job([this, num_slots, available_slots, info, attributes, out_id, status] {
			rage::rlTaskStatus our_status{};
			if (!g_hooking->get_original<hooks::advertise_session>()(0, num_slots, available_slots, attributes, -1, info, out_id, &our_status))
			{
				LOG(WARNING) << __FUNCTION__ ": advertise_session returned false for first advertisement";
				status->status = 2;
				return;
			}

			while (our_status.status == 1)
				script::get_current()->yield();

			if (our_status.status == 2)
			{
				LOG(WARNING) << __FUNCTION__ ": advertise_session failed for first advertisement";
				status->status = 2;
				return;
			}

			MatchmakingId primary_id = *out_id; // create a copy if the original memory gets deallocated
			std::uint32_t id_hash    = rage::joaat(primary_id.m_data1);

			// m_data1 is generated from m_data2 and m_data3
			m_multiplexed_sessions.emplace(id_hash, std::vector<MatchmakingId>{});

			// create multiplex advertisements
			for (int i = 0; i < (g.spoofing.multiplex_count - 1); i++)
			{
				g_fiber_pool->queue_job([this, num_slots, available_slots, info, attributes, id_hash, i] {
					rage::rlTaskStatus status;
					MatchmakingId multiplexed_id;
					if (!g_hooking->get_original<hooks::advertise_session>()(0, num_slots, available_slots, attributes, -1, info, &multiplexed_id, &status))
					{
						LOG(WARNING) << __FUNCTION__ ": advertise_session returned false for multiplex task " << i;
						return;
					}

					while (status.status == 1)
						script::get_current()->yield();

					if (status.status == 2)
					{
						LOG(WARNING) << __FUNCTION__ ": advertise_session failed for multiplex task " << i;
						return;
					}

					if (auto it = m_multiplexed_sessions.find(id_hash); it != m_multiplexed_sessions.end())
					{
						it->second.push_back(multiplexed_id);
					}
					else
					{
						LOG(WARNING) << __FUNCTION__ ": created a multiplexed session advertisement, but the primary advertisement no longer exists";
					}
				});
			}
			status->status = 3; // return success for original caller
		});

		return true;
	}

	void matchmaking_service::handle_update(int num_slots, int available_slots, rage::rlSessionInfo* info, MatchmakingAttributes* attributes, MatchmakingId* id)
	{
		patch_matchmaking_attributes(attributes);

		// this can be fire and forget, but it's probably a good idea to be notified if something goes wrong
		if (auto it = m_multiplexed_sessions.find(rage::joaat(id->m_data1)); it != m_multiplexed_sessions.end())
		{
			if (!g.spoofing.multiplex_session)
			{
				// option disabled mid-session
				return;
			}

			int i = 0;
			for (auto& multiplex_session : it->second)
			{
				g_fiber_pool->queue_job([&multiplex_session, num_slots, available_slots, info, attributes, i] {
					rage::rlTaskStatus status;
					if (!g_hooking->get_original<hooks::update_session_advertisement>()(0, &multiplex_session, num_slots, available_slots, info, attributes, &status))
					{
						LOG(WARNING) << __FUNCTION__ ": update_session_advertisement returned false for multiplex task " << i;
						return;
					}

					while (status.status == 1)
						script::get_current()->yield();

					if (status.status == 2)
					{
						LOG(WARNING) << __FUNCTION__ ": update_session_advertisement failed for multiplex task " << i;
						return;
					}
				});
				i++;
			}
		}
	}

	bool matchmaking_service::handle_unadvertise(MatchmakingId* id)
	{
		if (auto it = m_multiplexed_sessions.find(rage::joaat(id->m_data1)); it != m_multiplexed_sessions.end())
		{
			for (auto& multiplex_session : it->second)
			{
				g_hooking->get_original<hooks::unadvertise_session>()(0, &multiplex_session, nullptr);
			}

			m_multiplexed_sessions.erase(it);
			return false;
		}
		else
		{
			if (g.spoofing.multiplex_session)
			{
				for (auto& [_, children] : m_multiplexed_sessions)
				{
					for (auto& session : children)
					{
						if (session.m_data2 == id->m_data2 && session.m_data3 == id->m_data3)
						{
							return true; // prevent auto cleanup
						}
					}
				}
			}
		}

		return false;
	}

	void matchmaking_service::handle_session_detail_send_response(rage::rlSessionDetailMsg* msg)
	{
		if (msg->m_status == 5)
		{
			if (g.spoofing.increase_player_limit)
			{
				msg->m_detail.m_player_count                   = std::min(29,
                    g.spoofing.spoof_session_player_count ? g.spoofing.session_player_count : (int)msg->m_detail.m_player_count);
				msg->m_detail.m_spectator_count                = 0;
				msg->m_detail.m_session_config.m_public_slots  = 30;
				msg->m_detail.m_session_config.m_private_slots = 2;
			}
			patch_matchmaking_attributes(&msg->m_detail.m_session_config.m_matchmaking_attributes);
		}
		else
		{
			LOG(WARNING) << __FUNCTION__ ": sending fail code " << msg->m_status;
		}
	}
}
