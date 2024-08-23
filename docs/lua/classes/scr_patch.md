# Class: scr_patch

Class for patching GTA script functions.

## Constructors (1)

### `new(script_name, patch_name, pattern, offset, patch_)`

Adds a patch for the specified script.

- **Parameters:**
  - `script_name` (string): The name of the script.
  - `patch_name` (string): The name of the patch.
  - `pattern` (string): The pattern to scan for within the script.
  - `offset` (integer): The position within the pattern.
  - `patch_` (table): The bytes to be written into the script's bytecode.

**Example Usage:**
```lua
my_patch = scr_patch:new(script_name, patch_name, pattern, offset, patch_)
```

## Functions (2)

### `enable()`

Enables the script patch for the current instance. When a new instance is created, it will be enabled by default.

**Example Usage:**
```lua
scr_patch:enable()
```

### `disable()`

Disables the script patch for the current instance.

**Example Usage:**
```lua
scr_patch:disable()
```