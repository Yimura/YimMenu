#include "hooking/hooking.hpp"
#include "pointers.hpp"
#include "util/mobile.hpp"

namespace big
{
	float hooks::damage_vehicle(CVehicleDamage* thisptr, rage::CEntity* source, eDamageType damage_type, Hash weapon, float damage, rage::fvector3* position, rage::fvector3* dot_product, rage::fvector3* unk, std::uint32_t body_part, std::uint64_t damage_material, std::int32_t wheel_index, bool unk1, bool unk2, float radius, bool unk3, bool unk4, bool source_is_melee, bool unk5, bool max_damage)
	{
		/*if (g.vehicle.god_mode)
		{
			if (self::veh != 0)
			{
				if (const auto current_vehicle = g_local_player->m_vehicle)
				{
					if (current_vehicle->m_driver == g_local_player && &current_vehicle->m_vehicle_damage == thisptr)
					{
						LOG(INFO) << "damage_vehicle REJECTED";
						return 0.f;
					}
				}
			}
			if (const auto personal_vehicle = mobile::mechanic::get_personal_cvehicle())
			{
				if (&personal_vehicle->m_vehicle_damage == thisptr)
				{
					LOG(INFO) << "personal_vehicle damage_vehicle REJECTED";
					return 0.f;
				}
			}
		}*/
		return g_hooking->get_original<hooks::damage_vehicle>()(thisptr, source, damage_type, weapon, damage, position, dot_product, unk, body_part, damage_material, wheel_index, unk1, unk2, radius, unk3, unk4, source_is_melee, unk5, max_damage);
	}

	bool hooks::apply_deformation(CDeformation* thisptr, rage::fvector3* unk, rage::fvector3* unk2, rage::CEntity* source, bool set_all_deformations, bool break_glass)
	{
		/*if (g.vehicle.proof_collision)
		{
			if (self::veh != 0)
			{
				if (const auto current_vehicle = g_local_player->m_vehicle)
				{
					if (current_vehicle->m_driver == g_local_player && &current_vehicle->m_vehicle_damage.m_deformation == thisptr)
					{
						LOG(INFO) << "apply_deformation REJECTED";
						return false;
					}
				}
			}
			if (const auto personal_vehicle = mobile::mechanic::get_personal_cvehicle())
			{
				if (&personal_vehicle->m_vehicle_damage.m_deformation == thisptr)
				{
					LOG(INFO) << "personal_vehicle apply_deformation REJECTED";
					return false;
				}
			}
		}*/
		return g_hooking->get_original<hooks::apply_deformation>()(thisptr, unk, unk2, source, set_all_deformations, break_glass);
	}

	bool hooks::is_vehicle_invincible(CVehicleDamage* thisptr)
	{
		if (g.vehicle.proof_collision)
		{
			if (thisptr->m_vehicle->m_driver == g_local_player)
			{
				DWORD64 v13                 = (DWORD64)thisptr->m_vehicle->m_driver->m_net_object;
				*(__int16*)(v13 + 0x52A) = 500;
				//LOG(INFO) << "is_vehicle_invincible spoofed";
				return true;
			}
		}
		return g_hooking->get_original<hooks::is_vehicle_invincible>()(thisptr);
	}
}
