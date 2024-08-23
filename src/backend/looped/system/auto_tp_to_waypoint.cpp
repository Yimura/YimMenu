#include "backend/looped_command.hpp"
#include "natives.hpp"
#include "util/teleport.hpp"

namespace big
{
	class auto_tp_to_waypoint : looped_command
	{
		using looped_command::looped_command;

		virtual void on_tick() override
		{
			teleport::to_blip((int)BlipIcons::RADAR_WAYPOINT);
		}
	};

	auto_tp_to_waypoint g_auto_tp_to_waypoint("autotptowp", "BACKEND_LOOPED_AUTO_TP_TO_WAYPOINT", "BACKEND_LOOPED_AUTO_TP_TO_WAYPOINT_DESC", g.self.auto_tp);
}
