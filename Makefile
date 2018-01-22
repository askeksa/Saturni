
EXTERNAL := ../Andres
BUILD = build

CC := i686-w64-mingw32-g++
INCLUDES := -I$(EXTERNAL)/glfw-3.0.4.bin.WIN32/include -I$(EXTERNAL)/glew-1.10.0/include -I$(EXTERNAL)/portaudio/include -I$(EXTERNAL)/LuaJIT-2.0.4/src
LIBRARIES := $(EXTERNAL)/glew-1.10.0/lib/Release/Win32/glew32s.lib -L$(EXTERNAL)/glfw-3.0.4.bin.WIN32/lib-mingw $(EXTERNAL)/portaudio/mingw32/usr/local/lib/libportaudio-2.dll -L$(EXTERNAL)/LuaJIT-2.0.5/src
CFLAGS := $(INCLUDES) -Wno-write-strings -std=c++11
LFLAGS := $(LIBRARIES) -lluajit -lglfw3 -lopengl32 -luser32 -lgdi32 -static-libgcc -static-libstdc++

ifeq ($(DEBUG),yes)
CFLAGS += -g
else
CFLAGS += -O3
LFLAGS += -s
endif

$(BUILD)/saturni: $(patsubst %,$(BUILD)/%.o,main music lua_runner shader_runner)
	$(CC) $^ $(LFLAGS) -o $(BUILD)/saturni
	cp lib/* $(BUILD)/

$(BUILD)/%.o: src/%.cpp Makefile | $(BUILD)
	$(CC) $(CFLAGS) $< -c -o $@

$(BUILD)/main.o: src/main.cpp src/music.h src/filewatch.h src/lua_runner.h src/shader_runner.h

$(BUILD)/music.o: src/music.cpp src/music.h

$(BUILD)/lua_runner.o: src/lua_runner.cpp src/lua_runner.h

$(BUILD)/shader_runner.o: src/shader_runner.cpp src/shader_runner.h

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -f $(BUILD)/*
