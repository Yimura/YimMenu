# Table: gui

Table containing functions for modifying the menu GUI.

## Functions (12)

### `get_tab(tab_name)`

- **Parameters:**
  - `tab_name` (string): Name of the tab to get.

- **Returns:**
  - `tab`: A tab instance which corresponds to the tab in the GUI.

**Example Usage:**
```lua
tab = gui.get_tab(tab_name)
```

### `add_tab(tab_name)`

- **Parameters:**
  - `tab_name` (string): Name of the tab to add.

- **Returns:**
  - `tab`: A tab instance which corresponds to the new tab in the GUI.

**Example Usage:**
```lua
tab = gui.add_tab(tab_name)
```

### `show_success(title, message)`

Shows a success to the user with the given title and message.

- **Parameters:**
  - `title` (string)
  - `message` (string)

**Example Usage:**
```lua
gui.show_success(title, message)
```

### `show_message(title, message)`

Shows a message to the user with the given title and message.

- **Parameters:**
  - `title` (string)
  - `message` (string)

**Example Usage:**
```lua
gui.show_message(title, message)
```

### `show_warning(title, message)`

Shows a warning to the user with the given title and message.

- **Parameters:**
  - `title` (string)
  - `message` (string)

**Example Usage:**
```lua
gui.show_warning(title, message)
```

### `show_error(title, message)`

Shows an error to the user with the given title and message.

- **Parameters:**
  - `title` (string)
  - `message` (string)

**Example Usage:**
```lua
gui.show_error(title, message)
```

### `is_open()`

- **Returns:**
  - `bool`: Returns true if the GUI is open.

**Example Usage:**
```lua
bool = gui.is_open()
```

### `toggle(toggle)`

Opens and closes the gui.

- **Parameters:**
  - `toggle` (boolean)

**Example Usage:**
```lua
gui.toggle(toggle)
```

### `mouse_override()`

- **Returns:**
  - `bool`: Returns true if the mouse is overridden.

**Example Usage:**
```lua
bool = gui.mouse_override()
```

### `override_mouse(override)`

- **Parameters:**
  - `override` (boolean)

**Example Usage:**
```lua
gui.override_mouse(override)
```

### `add_imgui(imgui_rendering)`

Registers a function that will be called every rendering frame, you can call ImGui functions in it, please check the ImGui.md documentation file for more info.
**Example Usage:**
```lua
gui.add_imgui(function()
   if ImGui.Begin("My Custom Window") then
       if ImGui.Button("Label") then
         script.run_in_fiber(function(script)
           -- call natives in there
         end)
       end

       ImGui.End()
   end
end)
```

- **Parameters:**
  - `imgui_rendering` (function): Function that will be called every rendering frame, you can call ImGui functions in it, please check the ImGui.md documentation file for more info.

**Example Usage:**
```lua
gui.add_imgui(imgui_rendering)
```

### `add_always_draw_imgui(imgui_rendering)`

Registers a function that will be called every rendering frame, you can call ImGui functions in it, please check the ImGui.md documentation file for more info. This function will be called even when the menu is closed.
**Example Usage:**
```lua
gui.add_always_draw_imgui(function()
   if ImGui.Begin("My Custom Window") then
       if ImGui.Button("Label") then
         script.run_in_fiber(function(script)
           -- call natives in there
         end)
       end

       ImGui.End()
   end
end)
``

- **Parameters:**
  - `imgui_rendering` (function): Function that will be called every rendering frame, you can call ImGui functions in it, please check the ImGui.md documentation file for more info.

**Example Usage:**
```lua
gui.add_always_draw_imgui(imgui_rendering)
```


