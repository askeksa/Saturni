
#define CALLBACK __stdcall
#define GLEW_BUILD GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader_runner.h"

#include <cstdlib>
#include <cstring>
#include <vector>


struct QuadVertex {
	float x,y;
};

GLuint makeShader(GLenum kind, const char **source) {
	std::vector<char> log;
	GLsizei log_length;
	GLuint s = glCreateShader(kind);
	glShaderSource(s, 1, source, 0);
	glCompileShader(s);
	glGetShaderiv(s, GL_INFO_LOG_LENGTH, &log_length);
	log.resize(log_length + 1);
	glGetShaderInfoLog(s, log_length, nullptr, &log[0]);
	log[log_length] = '\0';
	printf("%s", &log[0]);
	fflush(stdout);
	return s;
}

GLuint makeProgram(const char *vsource, const char *psource) {
	std::vector<char> log;
	GLsizei log_length;
	GLuint program = glCreateProgram();
	GLuint vs = makeShader(GL_VERTEX_SHADER, &vsource);
	glAttachShader(program, vs);
	GLuint ps = makeShader(GL_FRAGMENT_SHADER, &psource);
	glAttachShader(program, ps);
	glLinkProgram(program);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
	log.resize(log_length + 1);
	glGetProgramInfoLog(program, log_length, nullptr, &log[0]);
	log[log_length] = '\0';
	printf("%s", &log[0]);
	fflush(stdout);
	return program;
}

static const char *quad_vshader = R"--(
#version 150

attribute vec2 xy;

varying vec2 uv;

void main() {
	gl_Position = vec4(xy * 2.0 - vec2(1.0), 0.0, 1.0);
    gl_Position.y = gl_Position.y * -1.0; // flip vertical right way around
	uv = xy;
}

)--";

static const char *scale_pshader = R"--(
#version 150

varying vec2 uv;

uniform sampler2D tex;

void main() {
	gl_FragColor = texture2D(tex, vec2(uv.x, 1.0 - uv.y));
}

)--";

ShaderRunner::ShaderRunner(int width, int height) : width(width), height(height) {
	// Make quad vertex buffer
	const float corners[3][2] = {
		{ 0.0, 0.0 },
		{ 2.0, 0.0 },
		{ 0.0, 2.0 },
	};
	glGenBuffers(1, &quad_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(QuadVertex), &corners[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Make render texture
	glGenTextures(1, &render_tex);
	glBindTexture(GL_TEXTURE_2D, render_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Make frame buffer
	glGenFramebuffers(1, &framebuf);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_tex, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete (%d)\n", status);
		fflush(stdout);
	}

	// Compile scale program
	scale_program = makeProgram(quad_vshader, scale_pshader);
	scale_xy_loc = glGetAttribLocation(scale_program, "xy");
}

ShaderRunner::~ShaderRunner() {
	glFinish();
	glDeleteProgram(scale_program);
	glDeleteProgram(user_program);
	glDeleteFramebuffers(1, &framebuf);
	glDeleteTextures(1, &render_tex);
	glDeleteBuffers(1, &quad_vertex_buffer);
}

void ShaderRunner::load(const char *filename) {
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Unable to open shader file '%s'\n", filename);
		fflush(stdout);
		return;
	}
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *shader_text = (char *) malloc(size + 1);
	fread(shader_text, 1, size, fp);
	shader_text[size] = '\0';
	fclose(fp);

	glDeleteProgram(user_program);
	user_program = makeProgram(quad_vshader, shader_text);
	free(shader_text);
	user_xy_loc = glGetAttribLocation(user_program, "xy");
}

void ShaderRunner::render(LuaRunner& lua) {
	// Save target state
	GLuint target;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &target);
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	// Render to FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuf);
	glViewport(0, 0, width, height);

	// Set user program
	glUseProgram(user_program);

	// Set up vertex streams for user shader
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glVertexAttribPointer(user_xy_loc, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), &((QuadVertex *)0)->x);
	glEnableVertexAttribArray(user_xy_loc);

	// Set uniforms
	GLint uniform_name_length;
	glGetProgramiv(user_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_name_length);
	if (uniform_name.size() < uniform_name_length) {
		uniform_name.resize(uniform_name_length);
	}
	char *name = &uniform_name[0];
	GLint n_uniforms;
	glGetProgramiv(user_program, GL_ACTIVE_UNIFORMS, &n_uniforms);
	for (GLint i = 0; i < n_uniforms; i++) {
		GLint size;
		GLenum type;
		float value[4];
		glGetActiveUniform(user_program, i, uniform_name.size(), nullptr,
			&size, &type, name);
		GLint loc = glGetUniformLocation(user_program, name);
		bool array = false;
		char *bracket = strstr(name, "[");
		if (bracket != nullptr) {
			array = true;
			*bracket = '\0';
		}
		for (int i = 0; i < size; i++) {
			int index = array ? i : -1;
			switch (type) {

#define TYPECASE(_gltype, _num) \
			case _gltype: \
				if (!lua.get_float_vec(name, index, _num, value)) { \
					std::fill(&value[0], &value[_num], 0.0f); \
				} \
				glUniform##_num##fv(loc + i, 1, value); \
				break;

			TYPECASE(GL_FLOAT, 1)
			TYPECASE(GL_FLOAT_VEC2, 2)
			TYPECASE(GL_FLOAT_VEC3, 3)
			TYPECASE(GL_FLOAT_VEC4, 4)
#undef TYPECASE

			default:
				// Unsupported uniform type
				break;
			}
		}
	}

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Cleanup
	glDisableVertexAttribArray(user_xy_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// Render to original render target
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target);
	glViewport(vp[0], vp[1], vp[2], vp[3]);

	// Set scale program
	glUseProgram(scale_program);

	// Set up vertex streams for scale shader
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glVertexAttribPointer(scale_xy_loc, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), &((QuadVertex *)0)->x);
	glEnableVertexAttribArray(scale_xy_loc);

	// Set texture
	GLint tex_loc = glGetUniformLocation(scale_program, "tex");
	glUniform1f(tex_loc, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, render_tex);

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Cleanup
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(scale_xy_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
