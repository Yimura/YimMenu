#include "translation_service.hpp"
#include "file_manager.hpp"
#include "thread_pool.hpp"
#include <cpr/cpr.h>

namespace big
{
    translation_service::translation_service() :
        m_url("https://raw.githubusercontent.com/YimMenu/Translations/master"),
        m_translation_directory(g_file_manager->get_project_folder("./translations"))
    {
        init();

        g_translation_service = this;
    }
    
    translation_service::~translation_service()
    {
        g_translation_service = nullptr;
    }

    std::string_view translation_service::get_translation(const std::string_view translation_key) const
    {
        return get_translation(rage::joaat(translation_key));
    }

    
    std::string_view translation_service::get_translation(const rage::joaat_t translation_key) const
    {
        if (auto it = m_translations.find(translation_key); it != m_translations.end())
            return it->second.c_str();

        return { 0, 0 };
    }

    std::map<std::string, translation_entry>& translation_service::available_translations()
    {
        return m_remote_index.translations;
    }

    const std::string& translation_service::current_language_pack()
    {
        return m_local_index.selected_language;
    }

    void translation_service::select_language_pack(const std::string& pack_id)
    {
        g_thread_pool->push([this, &pack_id]
        {
            update_local_index(m_remote_index.version, pack_id);
            load_translations();
        });
    }

    void translation_service::init()
    {
        bool loaded_remote_index = false;
        for (size_t i = 0; i < 5 && !loaded_remote_index; i++)
        {
            if (i)
                LOG(WARNING) << "Failed to download remote index, trying again... (" << i << ")";
            loaded_remote_index = download_index();
        }
        if (!loaded_remote_index)
        {
            LOG(WARNING) << "Failed to load remote index, unable to load translations.";
            return;
        }

        if (load_local_index())
        {
            if (m_local_index.version < m_remote_index.version)
            {
                LOG(INFO) << "Languages outdated, downloading new translations.";

                update_language_packs();
                update_local_index(m_remote_index.version);
            }
            load_translations();

            return;
        }

        LOG(INFO) << "Downloading translations...";

        download_language_pack(m_remote_index.default_lang);
        update_local_index(m_remote_index.version, m_remote_index.default_lang);

        load_translations();
    }

    void translation_service::load_translations()
    {
        m_translations.clear();

        // load default lang first to make sure there are fallback keys if another language pack doesn't have a certain key
        auto j = load_translation(m_remote_index.default_lang);
        for (auto& [key, value] : j.items())
        {
            m_translations.insert({ rage::joaat(key), value.get<std::string>() });
        }
        
        // Don't load selected language if it's the same as default
        if (m_local_index.selected_language == m_remote_index.default_lang)
            return;
        
        j = load_translation(m_local_index.selected_language);
        for (auto &[key, value] : j.items())
        {
            m_translations[rage::joaat(key)] = value;
        }
    }

    nlohmann::json translation_service::load_translation(const std::string_view pack_id)
    {
        auto file = m_translation_directory.get_file(std::format("./{}.json", pack_id));
        if (!file.exists())
        {
            LOG(INFO) << "Translations for '" << pack_id << "' do not exist, downloading...";
            download_language_pack(pack_id);
        }
        return nlohmann::json::parse(std::ifstream(file.get_path(), std::ios::binary));
    }

    bool translation_service::download_language_pack(const std::string_view pack_id)
    {
        if (auto it = m_remote_index.translations.find(pack_id.data()); it != m_remote_index.translations.end())
        {
            cpr::Response response = cpr::Get(cpr::Url{ m_url + "/" + it->second.file });

            if (response.status_code == 200)
            {
                auto json = nlohmann::json::parse(response.text);
                auto lang_file = m_translation_directory.get_file("./" + it->second.file);
                
                auto out_file = std::ofstream(lang_file.get_path(), std::ios::binary | std::ios::trunc);
                out_file << json.dump(4);
                out_file.close();

                return true;
            }
        }
        return false;
    }

    void translation_service::update_language_packs()
    {
        for (auto item : std::filesystem::directory_iterator(m_translation_directory.get_path()))
        {
            const auto path = item.path();
            const auto stem = path.stem().string();
            if (stem == "index" || item.path().extension() != ".json")
                continue;

            if (!download_language_pack(stem))
            {
                LOG(WARNING) << "Failed to update '" << stem << "' language pack";
            }
        }
    }

    bool translation_service::download_index()
    {
        cpr::Response response = cpr::Get(cpr::Url{ m_url + "/index.json" });

        if (response.status_code == 200)
        {
            m_remote_index = nlohmann::json::parse(response.text);

            return true;
        }
        return false;
    }

    bool translation_service::load_local_index()
    {
        const auto local_index = m_translation_directory.get_file("./index.json");
        if (local_index.exists())
        {
            const auto path = local_index.get_path();
            m_local_index = nlohmann::json::parse(std::ifstream(path, std::ios::binary));

            return true;
        }
        return false;
    }

    void translation_service::update_local_index(int version, std::string pack_id)
    {
        m_local_index.version = version;
        if (!pack_id.empty())
            m_local_index.selected_language = pack_id;
        nlohmann::json j = m_local_index;

        const auto local_index = m_translation_directory.get_file("./index.json");

        auto os = std::ofstream(local_index.get_path(), std::ios::binary | std::ios::trunc);
        os << j.dump(4);
    }

    std::string_view operator ""_T(const char* str, std::size_t len)
    {
        if (const auto translation = g_translation_service->get_translation(rage::joaat({ str, len })); translation.length())
            return translation;
        return { str, len };
    }
}