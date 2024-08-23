# Class: pointer

Class representing a 64-bit memory address.

## Constructors (1)

### `new(address)`

Returns a memory instance, with the given address.

- **Parameters:**
  - `address` (integer): Address

**Example Usage:**
```lua
myInstance = pointer:new(address)
```

## Functions (26)

### `add(offset)`

Adds an offset to the current memory address and returns a new pointer object.

- **Parameters:**
  - `offset` (integer): offset

- **Returns:**
  - `pointer`: new pointer object.

**Example Usage:**
```lua
pointer = pointer:add(offset)
```

### `sub(offset)`

Subs an offset to the current memory address and returns a new pointer object.

- **Parameters:**
  - `offset` (integer): offset

- **Returns:**
  - `pointer`: new pointer object.

**Example Usage:**
```lua
pointer = pointer:sub(offset)
```

### `rip(offset)`

Rips the current memory address and returns a new pointer object.

- **Parameters:**
  - `offset` (integer): offset

- **Returns:**
  - `pointer`: new pointer object.

**Example Usage:**
```lua
pointer = pointer:rip(offset)
```

### `get_byte()`

Retrieves the value stored at the memory address as the specified type.

- **Returns:**
  - `number`: the value stored at the memory address as the specified type.

**Example Usage:**
```lua
number = pointer:get_byte()
```

### `get_word()`

Retrieves the value stored at the memory address as the specified type.

- **Returns:**
  - `number`: the value stored at the memory address as the specified type.

**Example Usage:**
```lua
number = pointer:get_word()
```

### `get_int()`

Retrieves the value stored at the memory address as the specified type.

- **Returns:**
  - `number`: the value stored at the memory address as the specified type.

**Example Usage:**
```lua
number = pointer:get_int()
```

### `get_dword()`

Retrieves the value stored at the memory address as the specified type.

- **Returns:**
  - `number`: the value stored at the memory address as the specified type.

**Example Usage:**
```lua
number = pointer:get_dword()
```

### `get_float()`

Retrieves the value stored at the memory address as the specified type.

- **Returns:**
  - `float`: the value stored at the memory address as the specified type.

**Example Usage:**
```lua
float = pointer:get_float()
```

### `get_qword()`

Retrieves the value stored at the memory address as the specified type.

- **Returns:**
  - `number`: the value stored at the memory address as the specified type.

**Example Usage:**
```lua
number = pointer:get_qword()
```

### `set_byte(value)`

Sets the value at the memory address to the specified value of the given type.

- **Parameters:**
  - `value` (number): new value.

**Example Usage:**
```lua
pointer:set_byte(value)
```

### `set_word(value)`

Sets the value at the memory address to the specified value of the given type.

- **Parameters:**
  - `value` (number): new value.

**Example Usage:**
```lua
pointer:set_word(value)
```

### `set_int(value)`

Sets the value at the memory address to the specified value of the given type.

- **Parameters:**
  - `value` (number): new value.

**Example Usage:**
```lua
pointer:set_int(value)
```

### `set_dword(value)`

Sets the value at the memory address to the specified value of the given type.

- **Parameters:**
  - `value` (number): new value.

**Example Usage:**
```lua
pointer:set_dword(value)
```

### `set_float(value)`

Sets the value at the memory address to the specified value of the given type.

- **Parameters:**
  - `value` (float): new value.

**Example Usage:**
```lua
pointer:set_float(value)
```

### `set_qword(value)`

Sets the value at the memory address to the specified value of the given type.

- **Parameters:**
  - `value` (number): new value.

**Example Usage:**
```lua
pointer:set_qword(value)
```

### `get_string()`

Retrieves the value stored at the memory address as the specified type.

- **Returns:**
  - `string`: the value stored at the memory address as the specified type.

**Example Usage:**
```lua
string = pointer:get_string()
```

### `set_string(value)`

Sets the value at the memory address to the specified value of the given type.

- **Parameters:**
  - `value` (string): new value.

**Example Usage:**
```lua
pointer:set_string(value)
```

### `patch_byte(value)`

Creates a memory patch for modifying the value at the memory address with the specified value.
The modified value is applied when you call the apply function on the lua_patch object.
The original value is restored when you call the restore function on the lua_patch object.

- **Parameters:**
  - `value` (number): new value.

- **Returns:**
  - `lua_patch`: memory patch instance for modifying the value at the memory address with the specified value. Can call apply / restore on the object.

**Example Usage:**
```lua
lua_patch = pointer:patch_byte(value)
```

### `patch_word(value)`

Creates a memory patch for modifying the value at the memory address with the specified value.
The modified value is applied when you call the apply function on the lua_patch object.
The original value is restored when you call the restore function on the lua_patch object.

- **Parameters:**
  - `value` (number): new value.

- **Returns:**
  - `lua_patch`: memory patch instance for modifying the value at the memory address with the specified value. Can call apply / restore on the object.

**Example Usage:**
```lua
lua_patch = pointer:patch_word(value)
```

### `patch_dword(value)`

Creates a memory patch for modifying the value at the memory address with the specified value.
The modified value is applied when you call the apply function on the lua_patch object.
The original value is restored when you call the restore function on the lua_patch object.

- **Parameters:**
  - `value` (number): new value.

- **Returns:**
  - `lua_patch`: memory patch instance for modifying the value at the memory address with the specified value. Can call apply / restore on the object.

**Example Usage:**
```lua
lua_patch = pointer:patch_dword(value)
```

### `patch_qword(value)`

Creates a memory patch for modifying the value at the memory address with the specified value.
The modified value is applied when you call the apply function on the lua_patch object.
The original value is restored when you call the restore function on the lua_patch object.

- **Parameters:**
  - `value` (number): new value.

- **Returns:**
  - `lua_patch`: memory patch instance for modifying the value at the memory address with the specified value. Can call apply / restore on the object.

**Example Usage:**
```lua
lua_patch = pointer:patch_qword(value)
```

### `is_null()`

- **Returns:**
  - `boolean`: Returns true if the address is null.

**Example Usage:**
```lua
boolean = pointer:is_null()
```

### `is_valid()`

- **Returns:**
  - `boolean`: Returns true if the address is not null.

**Example Usage:**
```lua
boolean = pointer:is_valid()
```

### `deref()`

Dereferences the memory address and returns a new pointer object pointing to the value at that address.

- **Returns:**
  - `pointer`: A new pointer object pointing to the value at that address.

**Example Usage:**
```lua
pointer = pointer:deref()
```

### `get_address()`

Retrieves the memory address stored in the pointer object.

- **Returns:**
  - `number`: The memory address stored in the pointer object as a number.

**Example Usage:**
```lua
number = pointer:get_address()
```

### `set_address(address)`

Sets the memory address stored in the pointer object.

- **Parameters:**
  - `address` (integer): new address.

**Example Usage:**
```lua
pointer:set_address(address)
```


