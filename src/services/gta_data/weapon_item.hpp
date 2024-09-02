#pragma once

namespace big
{
	enum RPFDatafileSource : std::uint8_t
	{
		UNKNOWN,
		BASE,
		UPDATE,
		DLC,
		DLCUPDATE
	};

	class weapon_item
	{
	public:
		std::string m_name;
		std::string m_display_name;
		std::string m_display_desc;
		std::string m_weapon_type;
		uint32_t m_hash;
		uint32_t m_reward_hash;
		uint32_t m_reward_ammo_hash;
		std::vector<std::string> m_attachments;
		bool m_throwable;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(weapon_item, m_name, m_display_name, m_display_desc, m_weapon_type, m_hash, m_reward_hash, m_reward_ammo_hash, m_attachments, m_throwable)
	};

	class weapon_item_parsed : public weapon_item
	{
	public:
		RPFDatafileSource rpf_file_type = RPFDatafileSource::UNKNOWN;
	};
}