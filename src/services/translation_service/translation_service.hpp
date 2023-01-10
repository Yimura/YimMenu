#pragma once
#include "gta/joaat.hpp"
#include "file_manager/folder.hpp"
#include "local_index.hpp"
#include "remote_index.hpp"

namespace big
{
    using translation_map = std::unordered_map<rage::joaat_t, std::string>;

    class translation_service
    {
    public:
        translation_service();
        virtual ~translation_service() = default;
        translation_service(const translation_service&) = delete;
        translation_service(translation_service&&) noexcept  = delete;
        translation_service& operator=(const translation_service&) = delete;
        translation_service& operator=(translation_service&&) noexcept  = delete;
        
        void init();

        std::string_view get_translation(const std::string_view translation_key) const;
        std::string_view get_translation(const rage::joaat_t translation_key) const;

        std::map<std::string, translation_entry>& available_translations();
        const std::string& current_language_pack();
        void select_language_pack(const std::string& pack_id);

    private:
        void load_translations();
        nlohmann::json load_translation(const std::string_view pack_id);

        bool download_language_pack(const std::string_view pack_id);
        void update_language_packs();

        /**
         * @brief Downloads the remote index to compare with our local index
         * 
         * @return true 
         * @return false 
         */
        bool download_index();
        bool load_local_index();
        void update_local_index(int version, std::string pack_id = "");
    
    private:
        const std::string m_url;
        std::unique_ptr<folder> m_translation_directory;
        local_index m_local_index;
        remote_index m_remote_index;

        translation_map m_translations;

    };

    inline auto g_translation_service = translation_service();
    
    extern std::string_view operator ""_T(const char* str, std::size_t len);
}