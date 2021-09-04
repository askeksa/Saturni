#pragma once

#include <luajit-2.1/lua.hpp>

class LuaRunner {
	lua_State *L;
	bool valid = false;
	bool output = true;

	void print(const char *format, ...);

public:
	LuaRunner();
	~LuaRunner();

	void enable_output(bool output) {
		this->output = output;
	}

	void load(const char *filename);
	void run_frame(double time);
	double get_number(const char *name);
	const char *get_string(const char *name);
	bool get_float_vec(const char *name, int index, int size, float *value);
};
