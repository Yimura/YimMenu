#pragma once

namespace lua::vector
{
	// Lua API: Class
	// Name: vec3
	// Class representing a 3D vector.

	// Lua API: Constructor
	// Class: vec3
	// Param: x: float: x component of the vector.
	// Param: y: float: y component of the vector.
	// Param: z: float: z component of the vector.
	// Returns: vec3: a vector that contains the x, y, and z values.

	// Lua API: Field
	// Class: vec3
	// Field: x: float
	// x component of the vector.

	// Lua API: Field
	// Class: vec3
	// Field: y: float
	// y component of the vector.

	// Lua API: Field
	// Class: vec3
	// Field: z: float
	// z component of the vector.

	void bind(sol::state& state);
}