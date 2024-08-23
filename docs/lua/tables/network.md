# Table: network

Table containing helper functions for network related features.

## Functions (22)

### `trigger_script_event(bitset, _args)`

Call trigger_script_event (TSE)

- **Parameters:**
  - `bitset` (integer)
  - `_args` (table)

**Example Usage:**
```lua
network.trigger_script_event(bitset, _args)
```

### `is_session_started()`

Returns true if the local player is in a multiplayer session.

**Example Usage:**
```lua
network.is_session_started()
```

### `give_pickup_rewards(player, reward)`

Give the given pickup reward to the given player.

- **Parameters:**
  - `player` (integer): Index of the player.
  - `reward` (integer): Index of the reward pickup.

**Example Usage:**
```lua
network.give_pickup_rewards(player, reward)
```

### `set_player_coords(player_idx, x, y, z)`

Teleport the given player to the given position.

- **Parameters:**
  - `player_idx` (integer): Index of the player.
  - `x` (float): New x position.
  - `y` (float): New y position.
  - `z` (float): New z position.

**Example Usage:**
```lua
network.set_player_coords(player_idx, x, y, z)
```

### `set_all_player_coords(x, y, z)`

Teleport all players to the given position.

- **Parameters:**
  - `x` (float): New x position.
  - `y` (float): New y position.
  - `z` (float): New z position.

**Example Usage:**
```lua
network.set_all_player_coords(x, y, z)
```

### `get_selected_player()`

- **Returns:**
  - `integer`: Returns the index of the currently selected player in the GUI.

**Example Usage:**
```lua
integer = network.get_selected_player()
```

### `get_selected_database_player_rockstar_id()`

- **Returns:**
  - `integer`: Returns the rockstar id of the currently selected player in the GUI.

**Example Usage:**
```lua
integer = network.get_selected_database_player_rockstar_id()
```

### `flag_player_as_modder(player_idx, reason, custom_reason)`

Flags the given player as a modder in our local database.

- **Parameters:**
  - `player_idx` (integer): Index of the player.
  - `reason` (Infraction): Reason why the player is flagged as a modder, if the infraction is CUSTOM_REASON, then the custom_reason string is added in the local database. For a full list of the possible infraction reasons to use, please check the infraction page.
  - `custom_reason` (string): Optional, required only when the infraction is CUSTOM_REASON. The custom reason why the player is flagged as a modder.

**Example Usage:**
```lua
network.flag_player_as_modder(player_idx, reason, custom_reason)
```

### `is_player_flagged_as_modder(player_idx)`

- **Parameters:**
  - `player_idx` (integer): Index of the player.

- **Returns:**
  - `boolean`: Returns true if the given player is flagged as a modder.

**Example Usage:**
```lua
boolean = network.is_player_flagged_as_modder(player_idx)
```

### `is_player_friend(player_idx)`

- **Parameters:**
  - `player_idx` (integer): Index of the player.

- **Returns:**
  - `boolean`: Returns true if the given player is a friend.

**Example Usage:**
```lua
boolean = network.is_player_friend(player_idx)
```

### `get_flagged_modder_reason(player_idx)`

- **Parameters:**
  - `player_idx` (integer): Index of the player.

- **Returns:**
  - `string`: Returns a string which contains the reason(s) why the player is flagged as a modder.

**Example Usage:**
```lua
string = network.get_flagged_modder_reason(player_idx)
```

### `force_script_host(script_name)`

Try to force ourself to be host for the given GTA Script. Needs to be called in the fiber pool or a loop.

- **Parameters:**
  - `script_name` (string): Name of the script.

**Example Usage:**
```lua
network.force_script_host(script_name)
```

### `force_script_on_player(player_idx, script_name, instance_id)`

Forces the given GTA script to be started on a player. Needs to be called in the fiber pool or a loop.

- **Parameters:**
  - `player_idx` (integer): Index of the player.
  - `script_name` (string): Name of the script.
  - `instance_id` (integer): Instance ID of the script.

**Example Usage:**
```lua
network.force_script_on_player(player_idx, script_name, instance_id)
```

### `send_chat_message(msg, team_only)`

Sends a message to the in game chat.

- **Parameters:**
  - `msg` (string): Message to be sent.
  - `team_only` (boolean): Should be true if the msg should only be sent to our team.

**Example Usage:**
```lua
network.send_chat_message(msg, team_only)
```

### `send_chat_message_to_player(player_idx, msg)`

Sends a chat message to the specified player. Other players would not be able to see the message

- **Parameters:**
  - `player_idx` (integer): Index of the player.
  - `msg` (string): Message to be sent.

**Example Usage:**
```lua
network.send_chat_message_to_player(player_idx, msg)
```

### `get_player_rank(pid)`

Call get_player_rank(playerID)

- **Parameters:**
  - `pid` (integer): Index of the player.

- **Returns:**
  - `integer`: An integer which contains the players rank.

**Example Usage:**
```lua
integer = network.get_player_rank(pid)
```

### `get_player_rp(pid)`

Call get_player_rp(playerID)

- **Parameters:**
  - `pid` (integer): Index of the player.

- **Returns:**
  - `integer`: An integer which contains the players rp.

**Example Usage:**
```lua
integer = network.get_player_rp(pid)
```

### `get_player_money(pid)`

Call get_player_money(playerID)

- **Parameters:**
  - `pid` (integer): Index of the player.

- **Returns:**
  - `integer`: An integer which contains the players money.

**Example Usage:**
```lua
integer = network.get_player_money(pid)
```

### `get_player_wallet(pid)`

Call get_player_wallet(playerID)

- **Parameters:**
  - `pid` (integer): Index of the player.

- **Returns:**
  - `integer`: An integer which contains the players wallet.

**Example Usage:**
```lua
integer = network.get_player_wallet(pid)
```

### `get_player_bank(pid)`

Call get_player_bank(playerID)

- **Parameters:**
  - `pid` (integer): Index of the player.

- **Returns:**
  - `integer`: An integer which contains the players bank.

**Example Usage:**
```lua
integer = network.get_player_bank(pid)
```

### `get_player_language_id(pid)`

Call get_player_language_id(playerID)

- **Parameters:**
  - `pid` (integer): Index of the player.

- **Returns:**
  - `integer`: An integer which contains the players language id.

**Example Usage:**
```lua
integer = network.get_player_language_id(pid)
```

### `get_player_language_name(pid)`

Call get_player_language_name(playerID)

- **Parameters:**
  - `pid` (integer): Index of the player.

- **Returns:**
  - `string`: A string which contains the players language name.

**Example Usage:**
```lua
string = network.get_player_language_name(pid)
```


