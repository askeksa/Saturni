
#include "lua_runner.h"

#include <cstdarg>


void error(const char *msg) {
	fprintf(stderr, "ERROR: %s\n", msg);
	fflush(stderr);
}

#define CHECK(c) if ((c) != 0) error(lua_tostring(L, -1))


void LuaRunner::print(const char *format, ...) {
	if (output) {
		va_list va;
		va_start(va, format);
		vprintf(format, va);
		fflush(stdout);
		va_end(va);
	}
}


LuaRunner::LuaRunner() {
	L = lua_open();
	luaL_openlibs(L);

	lua_newtable(L);
	lua_setglobal(L, "persistent");
}

LuaRunner::~LuaRunner() {
	lua_close(L);
}

void LuaRunner::load(const char *filename) {
	valid = true;
	CHECK(luaL_loadfile(L, filename));
	if (!valid) return;
	CHECK(lua_pcall(L, 0, 0, 0));
	if (!valid) return;
}

void LuaRunner::run_frame(double time) {
	if (!valid) return;
	lua_getglobal(L, "frame");
	if (!lua_isfunction(L, -1)) {
		print("No global function named '%s'!\n", "frame");
		lua_pop(L, 1);
	} else {
		lua_pushnumber(L, time);
		CHECK(lua_pcall(L, 1, 0, 0));
	}
}

double LuaRunner::get_number(const char *name) {
	double value = 0.0;
	if (!valid) return value;
	lua_getglobal(L, name);
	if (lua_isnil(L, -1)) {
		print("No global variable named '%s'!\n", name);
	} else if (!lua_isnumber(L, -1)) {
		print("Global variable '%s' does not contain a number!\n", name);
	} else {
		value = lua_tonumber(L, -1);
	}
	lua_pop(L, 1);
	return value;
}

const char *LuaRunner::get_string(const char *name) {
	const char *value = nullptr;
	if (!valid) return value;
	lua_getglobal(L, name);
	if (lua_isnil(L, -1)) {
		print("No global variable named '%s'!\n", name);
	} else if (!lua_isstring(L, -1)) {
		print("Global variable '%s' does not contain a string!\n", name);
	} else {
		value = lua_tostring(L, -1);
	}
	lua_pop(L, 1);
	return value;
}
