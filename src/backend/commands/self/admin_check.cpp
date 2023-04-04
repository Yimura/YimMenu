#include "backend/command.hpp"
#include "core/globals.hpp"

#include <algorithm>
#include <vector>


namespace big
{
	class admin_check : command
	{
		using looped_command::command;
		// taken from https://github.com/RockstarGamesAdminsStaffGamertags/List and prs
		std::vector<std::uint64_t> admin_names = {67241866, 89288299, 88439202, 179848415, 184360405, 184359255, 182860908, 117639172, 142582982, 115642993, 100641297, 116815567, 88435319, 64499496, 174623946, 174626867, 151972200, 115643538, 144372813, 88047835, 115670847, 173426004, 170727774, 93759254, 174247774, 151975489, 146999560, 179930265, 88435236, 179936743, 179848203, 151158634, 174623904, 179936852, 117639190, 93759401, 103814653, 121970978, 174623951, 174624061, 10552062, 174625194, 174625307, 174625407, 174625552, 174625647, 138273823, 138302559, 139813495, 88435916, 174875493, 171094021, 173213117, 171093866, 88435362, 137601710, 103054099, 104041189, 99453882, 104432921, 147604980, 130291558, 141884823, 131037988, 153219155, 155527062, 114982881, 119266383, 119958356, 216820, 121397532, 121698158, 18965281, 56778561, 63457, 121943600, 123017343, 123849404, 127448079, 129159629, 127403483, 174194059, 131973478, 64234321, 62409944, 64074298, 133709045, 134412628, 137579070, 137714280, 137851207, 130291511, 138075198, 137663665, 9284553, 147111499, 6597634, 23659342, 23659354, 103318524, 132521200, 107713114, 107713060, 23659353, 57233573, 111439945, 81691532, 77205006, 25695975, 24646485, 49770174, 146452200, 54468359, 54462116, 53309582, 85593421, 21088063, 50850475, 31586721, 56583239, 20158753, 20158751, 23659351, 91031119, 91003708, 16396170, 16396157, 16396148, 16396141, 16396133, 16396126, 16396118, 16396107, 16396096, 16396091, 16396080, 16395850, 16395840, 16395850, 16395782, 16395773, 22577458, 22577440, 22577121, 16395782, 20158757, 20158757, 117641111, 173200071, 196222661, 179654627, 214327469, 213560223, 179608067, 209260139, 195489237, 209260788, 201693153, 211750362, 211702584, 211532217, 24037237, 77817603, 205951017, 89705641, 179848153, 193975449, 201727585, 201693153, 193972138, 192796203, 201693551, 204071275, 67241866, 174754789, 135811063, 64624133, 89705672, 89797943, 191415974, 28776717, 41352312, 56176623};
		virtual void on_tick() override
		{
			g_player_service->iterate([&](const player_entry& plyr) {
				auto found = std::find(admin_names.begin(), admin_names.end(), std::to_string(plyr.second->id()));
				if (found != admin_names.end())
				{
					g_notification_service->push_warning("Potential Admin Found!", "A potential admin has been found in your lobby!");
				}
			});
		};
	};

	admin_check g_admin_check("admin_check", "Checks for Admins", "Scans players in the lobby for potential Rockstar Admins", 0);
}