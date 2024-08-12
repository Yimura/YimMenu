# Table: menu_event

Table containing all possible events to which you can respond.

## Fields (9)

### `PlayerLeave`

Event that is triggered when a player leave the game session.
**Example Usage:**
```lua
event.register_handler(menu_event.PlayerLeave, function (player_name)
    log.info(player_name)
end)
```

- Type: `integer`

### `PlayerJoin`

Event that is triggered when a player join the game session.
**Example Usage:**
```lua
event.register_handler(menu_event.PlayerJoin, function (player_name, player_id)
    log.info(player_name)
    log.info(player_id)
end)
```

- Type: `integer`

### `PlayerMgrInit`

Event that is triggered when the player manager initialize. Usually called when we are joining a session.
**Example Usage:**
```lua
event.register_handler(menu_event.PlayerMgrInit, function ()
    log.info("Player manager inited, we just joined a session.")
end)
```

- Type: `integer`

### `PlayerMgrShutdown`

Event that is triggered when the player manager shutdown. Usually called when we are leaving a session.
**Example Usage:**
```lua
event.register_handler(menu_event.PlayerMgrShutdown, function ()
    log.info("Player manager inited, we just left a session.")
end)
```

- Type: `integer`

### `ChatMessageReceived`

Event that is triggered when we receive a in-game chat message.
**Example Usage:**
```lua
event.register_handler(menu_event.ChatMessageReceived, function (player_id, chat_message)
    log.info(player_id)
    log.info(chat_message)
end)
```

- Type: `integer`

### `ScriptedGameEventReceived`

Event that is triggered when we receive a scripted game event.
**Example Usage:**
```lua
event.register_handler(menu_event.ScriptedGameEventReceived, function (player_id, script_event_args)
    log.info(player_id)
    log.info(script_event_args)
end)
```

- Type: `integer`

### `MenuUnloaded`

Event that is triggered when we unload YimMenu.
**Example Usage:**
```lua
event.register_handler(menu_event.MenuUnloaded, function ()
    log.info("Menu unloaded.")
end)
```

- Type: `integer`

### `ScriptsReloaded`

Event that is triggered when we reload the Lua scripts.
**Example Usage:**
```lua
event.register_handler(menu_event.ScriptsReloaded, function ()
    log.info("Scripts reloaded.")
end)
```

- Type: `integer`

### `Wndproc`

Event that is triggered when Wndproc is called
**Example Usage:**
```lua
event.register_handler(menu_event.Wndproc, function (hwnd, msg, wparam, lparam)
    if msg == 132 then return end
    log.debug("hwnd = " .. tostring(hwnd) .. ", msg = " .. tostring(msg) .. ", wparam = " .. tostring(wparam) .. ", lparam = " .. tostring(lparam))
end)
```

- Type: `integer`

