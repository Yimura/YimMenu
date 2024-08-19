# Class: tab

Class for representing a tab within the GUI.

## Functions (12)

### `is_selected()`

- **Returns:**
  - `boolean`: Returns true if this tab is the one currently selected in the GUI.

**Example Usage:**
```lua
boolean = tab:is_selected()
```

### `clear()`

Clear the tab of all its custom lua content that you own.

**Example Usage:**
```lua
tab:clear()
```

### `add_tab(tab_name)`

Add a sub tab to this tab.

- **Parameters:**
  - `tab_name` (string): Name of the tab to add.

- **Returns:**
  - `tab`: A tab instance which corresponds to the new tab in the GUI.

**Example Usage:**
```lua
tab = tab:add_tab(tab_name)
```

### `add_button(name, callback)`

Add a button to the gui tab.

- **Parameters:**
  - `name` (string): Text written inside the button.
  - `callback` (function): function that will be called when the button is clicked.

**Example Usage:**
```lua
tab:add_button(name, callback)
```

### `add_text(name)`

Add text to the gui tab.

- **Parameters:**
  - `name` (string): Text that will be written.

- **Returns:**
  - `text`: The text object instance.

**Example Usage:**
```lua
text = tab:add_text(name)
```

### `add_checkbox(name)`

Add a checkbox widget to the gui tab.

- **Parameters:**
  - `name` (string): Text that will be written next to the checkbox.

- **Returns:**
  - `checkbox`: The checkbox object instance.

**Example Usage:**
```lua
checkbox = tab:add_checkbox(name)
```

### `add_sameline()`

Add a ImGui::SameLine.

- **Returns:**
  - `sameline`: The sameline object instance.

**Example Usage:**
```lua
sameline = tab:add_sameline()
```

### `add_separator()`

Add a ImGui::Separator.

- **Returns:**
  - `separator`: The separator object instance.

**Example Usage:**
```lua
separator = tab:add_separator()
```

### `add_input_int(name)`

Add a ImGui::InputInt.

- **Parameters:**
  - `name` (string): Text that will be written next to the input field.

- **Returns:**
  - `input_int`: The input_int object instance.

**Example Usage:**
```lua
input_int = tab:add_input_int(name)
```

### `add_input_float(name)`

Add a ImGui::InputFloat.

- **Parameters:**
  - `name` (string): Text that will be written next to the input field.

- **Returns:**
  - `input_float`: The input_float object instance.

**Example Usage:**
```lua
input_float = tab:add_input_float(name)
```

### `add_input_string(name)`

Add a ImGui::InputText.

- **Parameters:**
  - `name` (string): Text that will be written next to the input field.

- **Returns:**
  - `input_string`: The input_string object instance.

**Example Usage:**
```lua
input_string = tab:add_input_string(name)
```

### `add_imgui(imgui_rendering)`

Registers a function that will be called every rendering frame, you can call ImGui functions in it, please check the ImGui.md documentation file for more info.
**Example Usage:**
```lua
tab:add_imgui(function()
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
tab:add_imgui(imgui_rendering)
```


