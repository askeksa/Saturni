
#define GLEW_BUILD GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader_runner.h"

#include <cstdlib>
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

ShaderRunner::ShaderRunner() {
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
}

ShaderRunner::~ShaderRunner() {
	glFinish();
	glDeleteBuffers(1, &quad_vertex_buffer);
}

static const char *quad_vshader = R"--(
#version 150

attribute vec2 xy;

varying vec2 uv;

void main() {
	gl_Position = vec4(xy.x * 2.0 - 1.0, 1.0 - xy.y * 2.0, 0.0, 1.0);
	uv = xy;
}

)--";

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

	program = makeProgram(quad_vshader, shader_text);
	free(shader_text);
	xy_loc = glGetAttribLocation(program, "xy");
}

void ShaderRunner::render(LuaRunner& lua) {
	// Set up vertex streams
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glVertexAttribPointer(xy_loc, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), &((QuadVertex *)0)->x);
	glEnableVertexAttribArray(xy_loc);

	// Set program
	glUseProgram(program);

	// Set uniforms
	GLint uniform_name_length;
	glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_name_length);
	if (uniform_name.size() < uniform_name_length) {
		uniform_name.resize(uniform_name_length);
	}
	GLint n_uniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &n_uniforms);
	for (GLint i = 0; i < n_uniforms; i++) {
		GLint size;
		GLenum type;
		glGetActiveUniform(program, i, uniform_name.size(), nullptr,
			&size, &type, &uniform_name[0]);
		double value = lua.get_number(&uniform_name[0]);
		GLint loc = glGetUniformLocation(program, &uniform_name[0]);
		glUniform1f(loc, value);
	}

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Cleanup
	glDisableVertexAttribArray(xy_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
