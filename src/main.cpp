
#define GLEW_BUILD GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <queue>
#include <cstdio>
#include <cstdlib>

#include "music.h"
#include "filewatch.h"
#include "lua_runner.h"
#include "shader_runner.h"

#define WIDTH 640
#define HEIGHT 480
#define FRAMERATE 60
#define FRAMES 10000



void error_callback(int error, const char* description) {
	printf(" *** GLFW error: %s\n", description);
	fflush(stdout);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	std::queue<int>* key_queue = (std::queue<int>*) glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		key_queue->push(key);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: saturni <lua file> [<scale>]\n");
		exit(1);
	}

	int scale = 1;
	if (argc > 2) {
		scale = atoi(argv[2]);
	}

	// Load code
	LuaRunner lua;
	FileWatch lua_file(argv[1]);
	lua.load(lua_file.name());

	int width = (int) lua.get_number("width");
	if (width == 0) width = WIDTH;

	int height = (int) lua.get_number("height");
	if (height == 0) height = HEIGHT;

	int framerate = (int) lua.get_number("framerate");
	if (framerate == 0) framerate = FRAMERATE;

	const char *title = lua.get_string("title");
	if (title == nullptr) title = "Saturni";

	MusicPlayer player;
	int frames = FRAMES;
	const char *music_file = lua.get_string("music");
	if (music_file) {
		player.load(music_file);
		frames = (int) (player.length() * framerate);
	}

	// Initialize GLFW
	glfwSetErrorCallback(error_callback);
	glfwInit();

	// Initialize Window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow *window = glfwCreateWindow(width * scale, height * scale, title, nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	glfwSwapInterval(1);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Shader runner and file
	ShaderRunner shader(width, height);
	FileWatch shader_file("");

	// Set up key callback
	std::queue<int> key_queue;
	glfwSetWindowUserPointer(window, &key_queue);
	glfwSetKeyCallback(window, key_callback);

	player.start(0);
	int startframe = 0;
	int frame = 0;
	bool playing = true;
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window)) {
		bool frame_set = false;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
			double xpos,ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			int wwidth,wheight;
			glfwGetWindowSize(window, &wwidth, &wheight);
			frame = (int)(xpos / wwidth * frames);
			frame_set = true;
			startframe = frame;
		}

		while (!key_queue.empty()) {
			int key = key_queue.front();
			key_queue.pop();
			switch (key) {
			case GLFW_KEY_SPACE:
				playing = !playing;
				if (playing) {
					startframe = frame;
					player.start(frame / (double) framerate);
				} else {
					player.stop();
					frame_set = true;
				}
				break;
			case GLFW_KEY_BACKSPACE:
				frame = startframe;
				frame_set = true;
				break;
			case GLFW_KEY_LEFT:
				frame -= 1;
				frame_set = true;
				break;
			case GLFW_KEY_RIGHT:
				frame += 1;
				frame_set = true;
				break;
			case GLFW_KEY_PAGE_UP:
				frame -= 50;
				frame_set = true;
				break;
			case GLFW_KEY_PAGE_DOWN:
				frame += 50;
				frame_set = true;
				break;
			case GLFW_KEY_HOME:
				frame = 0;
				frame_set = true;
				break;
			}
		}

		// Clamp frame
		if (frame < 0) frame = 0;
		if (frame > frames-1) frame = frames-1;

		// Reload Lua file if changed
		if (lua_file.changed()) {
			// Reload code
			printf("Loading Lua file '%s' at %s\n", lua_file.name(), lua_file.time_text());
			fflush(stdout);
			lua.enable_output(true);
			lua.load(lua_file.name());
			lua.enable_output(true);
			if (playing) {
				frame = startframe;
				frame_set = true;
			}
		}

		// Run
		lua.run_frame(frame / (double) framerate);

		// Reload shader file if changed
		const char *new_shader_file = lua.get_string("shader");
		bool shader_filename_changed = new_shader_file != nullptr &&
			strcmp(new_shader_file, shader_file.name()) != 0;
		if (shader_file.changed() || shader_filename_changed) {
			if (shader_filename_changed) {
				shader_file = new_shader_file;
			}
			printf("Loading shader file '%s' at %s\n", shader_file.name(), shader_file.time_text());
			fflush(stdout);
			shader.load(shader_file.name());
			lua.enable_output(true);
		}

		if (frame_set) {
			player.set_time(frame / (double) framerate);
		}

		// Clear
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClearColor(1,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Render
		shader.render(lua);

		glfwSwapBuffers(window);
		glfwPollEvents();

		lua.enable_output(false);

		if (playing) {
			int prev_frame = frame;
			do {
				usleep(1000);
				frame = (int)(player.get_time() * framerate);
			} while (frame == prev_frame);
		} else {
			usleep(100000);
		}
	}


	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

