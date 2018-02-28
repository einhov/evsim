#include <optional>
#include <string_view>
#include <vector>
#include "gfx_program.h"

namespace gfx {
	void program::attach(const std::string_view source, shader_type type) {
		const GLuint shader = [type] {
			switch(type) {
				case shader_type::FRAGMENT:
					return glCreateShader(GL_FRAGMENT_SHADER);
				case shader_type::VERTEX:
					return glCreateShader(GL_VERTEX_SHADER);
				default:
					return 0u;
			}
		}();

		if(shader == 0) return;

		const GLchar * const sources[1] = { source.data() };
		glShaderSource(shader, 1, sources, nullptr);
		glCompileShader(shader);

		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if(!status) {
			GLint length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			std::vector<GLchar> log(length);
			glGetShaderInfoLog(shader, length, nullptr, log.data());
			fprintf(stderr, "%s\n", log.data());
		}

		glAttachShader(program_ref, shader);
		glDeleteShader(shader);
	}

	void program::link() {
		glLinkProgram(program_ref);

		GLint status;
		glGetProgramiv(program_ref, GL_LINK_STATUS, &status);
		if(!status) {
			GLint length;
			glGetProgramiv(program_ref, GL_INFO_LOG_LENGTH, &length);
			std::vector<GLchar> log(length);
			glGetProgramInfoLog(program_ref, length, nullptr, log.data());
			fprintf(stderr, "%s\n", log.data());
		}

		GLint amount, name_length, size;
		GLenum type;

		glGetProgramiv(program_ref, GL_ACTIVE_UNIFORMS, &amount);
		glGetProgramiv(program_ref, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_length);

		if(amount <= 0) return;

		std::vector<GLchar> name(name_length);
		for(int i = 0; i < amount; i++) {
			glGetActiveUniform(program_ref, i, name_length, nullptr, &size, &type, name.data());
			uniforms.emplace(name.data(), i);
		}

	}

	template <>
	std::optional<GLfloat> program::get_uniform<GLfloat>(const std::string_view name) const {
		const auto uniform = uniforms.find(name);
		if(uniform == uniforms.cend()) return std::nullopt;
		GLfloat result;
		glGetnUniformfv(program_ref, uniform->second, 1 * sizeof(result), &result);
		return std::make_optional(result);
	}

	template <>
	bool program::set_uniform_array<program::uniform_type::FLOAT>(const std::string_view name, const GLfloat *values, size_t size) {
		const auto uniform = uniforms.find(name);
		while(glGetError() != GL_NO_ERROR);
		if(uniform != uniforms.cend()) {
			glUseProgram(program_ref);
			glUniform1fv(uniform->second, size, values);
			return true;
		} else {
			return false;
		}
	}

	template <>
	bool program::set_uniform_array<program::uniform_type::FLOAT2>(const std::string_view name, const GLfloat *values, size_t size) {
		const auto uniform = uniforms.find(name);
		while(glGetError() != GL_NO_ERROR);
		if(uniform != uniforms.cend()) {
			glUseProgram(program_ref);
			glUniform2fv(uniform->second, size, values);
			return true;
		} else {
			return false;
		}
	}

	template <>
	bool program::set_uniform_array<program::uniform_type::FLOAT3>(const std::string_view name, const GLfloat *values, size_t size) {
		const auto uniform = uniforms.find(name);
		while(glGetError() != GL_NO_ERROR);
		if(uniform != uniforms.cend()) {
			glUseProgram(program_ref);
			glUniform3fv(uniform->second, size, values);
			return true;
		} else {
			return false;
		}
	}

	template <>
	bool program::set_uniform_array<program::uniform_type::FLOAT4>(const std::string_view name, const GLfloat *values, size_t size) {
		const auto uniform = uniforms.find(name);
		while(glGetError() != GL_NO_ERROR);
		if(uniform != uniforms.cend()) {
			glUseProgram(program_ref);
			glUniform4fv(uniform->second, size, values);
			return true;
		} else {
			return false;
		}
	}

	template <>
	bool program::set_uniform_array<program::uniform_type::MAT4>(const std::string_view name, const GLfloat *values, size_t size) {
		const auto uniform = uniforms.find(name);
		while(glGetError() != GL_NO_ERROR);
		if(uniform != uniforms.cend()) {
			glUseProgram(program_ref);
			glUniformMatrix4fv(uniform->second, size, false, values);
			return true;
		} else {
			return false;
		}
	}
};
