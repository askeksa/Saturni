#pragma once

#include "lua_runner.h"

#include <vector>

class ShaderRunner {
	GLuint quad_vertex_buffer;
	GLuint program;
	GLuint xy_loc;

	std::vector<char> uniform_name;
public:
	ShaderRunner();
	~ShaderRunner();

	void load(const char *filename);
	void render(LuaRunner& lua);
};
