#pragma once

#include "lua_runner.h"

#include <vector>

class ShaderRunner {
	int width;
	int height;

	GLuint quad_vertex_buffer;
	GLuint render_tex;
	GLuint framebuf;
	GLuint scale_program = 0;
	GLuint scale_xy_loc;
	GLuint user_program = 0;
	GLuint user_xy_loc;

	std::vector<char> uniform_name;
public:
	ShaderRunner(int width, int height);
	~ShaderRunner();

	void load(const char *filename);
	void render(LuaRunner& lua);
};
