# Table: scr_function

Table for calling GTA script functions. Needs to be called in the fiber pool. Only call the function when necessary.

## Functions (2)

### `call_script_function(script_name, function_name, pattern, return_type_string, args_)`

Calls a script function with the given arguments. Returns the return value as the given type.
**Example Usage:**
```lua
local value = scr_function.call_script_function("freemode", "wear_sunglasses_at_night", "69 42 06 66", "bool", {
   { "int", 69 },
   { "float", 4.20 },
   { "int", 666 }
})
```

- **Parameters:**
  - `script_name` (string): Name of the script.
  - `function_name` (string): Name of the function. This parameter needs to be unique.
  - `pattern` (string): Pattern to scan for within the script.
  - `return_type_string` (string): Return type of the function. Supported types are **"int"**, **"bool"**, **"const char\*/string"**, **"ptr/pointer/*"**, **"float"**, and **"vector3"**. Anything different will be rejected.
  - `args_` (table): Arguments to pass to the function. Supported types are the same as return types.

**Example Usage:**
```lua
scr_function.call_script_function(script_name, function_name, pattern, return_type_string, args_)
```

### `call_script_function(script_name, instruction_pointer, return_type_string, args_)`

Calls a script function directly using the function position with the given arguments. Returns the return value as the given type.
**Example Usage:**
```lua
local value = scr_function.call_script_function("freemode", 0xE792, "string", {
   { "int", 191 }
})
```

- **Parameters:**
  - `script_name` (string): Name of the script.
  - `instruction_pointer` (integer): Position of the function within the script.
  - `return_type_string` (string): Return type of the function. Supported types are **"int"**, **"bool"**, **"const char\*/string"**, **"ptr/pointer/*"**, **"float"**, and **"vector3"**. Anything different will be rejected.
  - `args_` (table): Arguments to pass to the function. Supported types are the same as return types.

**Example Usage:**
```lua
scr_function.call_script_function(script_name, instruction_pointer, return_type_string, args_)
```


