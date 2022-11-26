#pragma once
#include "common.hpp"
#include "hotkey.hpp"

namespace big
{
    enum eKeyState : unsigned int
    {
        RELEASE = WM_KEYUP,
        DOWN = WM_KEYDOWN
    };

    using hotkey_map = std::map<key_t, hotkey>;

    class hotkey_service final
    {
    public:
        hotkey_service();
        virtual ~hotkey_service();
        hotkey_service(const hotkey_service&) = delete;
        hotkey_service(hotkey_service&&) noexcept  = delete;
        hotkey_service& operator=(const hotkey_service&) = delete;
        hotkey_service& operator=(hotkey_service&&) noexcept  = delete;


        void register_hotkey(const std::string_view name, const key_t initial_key, const hotkey_func func, const eKeyState state = eKeyState::RELEASE);
        bool update_hotkey(const std::string_view name, const key_t new_key);
        void wndproc(eKeyState state, key_t key);

    private:
        // yes curse me
        std::array<hotkey_map, 2> m_hotkeys;

    };

    inline hotkey_service* g_hotkey_service{};
}
