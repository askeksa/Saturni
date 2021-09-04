
BUILD = build

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
LIBRARIES := -lluajit-5.1 -lglfw -lGL -lGLEW -lportaudio
else
INCLUDES := -I/mingw64/include
LIBRARIES := -L/mingw64/lib -lluajit-5.1 -lopengl32 -lglfw3 -lglew32 -lportaudio
endif

CC := g++
CFLAGS := $(INCLUDES) -Wno-write-strings -std=c++11
LFLAGS := $(LIBRARIES)

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
