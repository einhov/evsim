#ifndef GFX_PROGRAM_H
#define GFX_PROGRAM_H

#include <optional>
#include <string_view>
#include <map>
#include <array>
#include <type_traits>
#include <functional>
#include <GL/glew.h>

namespace gfx {
	class program {
	public:
		enum class shader_type {
			FRAGMENT, VERTEX
		};

		program() : program_ref(glCreateProgram()) { }

		void attach(const std::string_view source, shader_type type);
		void link();
		void activate() const {
			glUseProgram(program_ref);
		}

		enum class uniform_type {
			FLOAT, FLOAT2, FLOAT3, FLOAT4,
			INT, INT2, INT3, INT4,
			UINT, UINT2, UINT3, UINT4,
			MAT2, MAT3, MAT4, MAT2X3, MAT3X2, MAT2X4, MAT4X2, MAT3X4, MAT4X3
		};

		template <uniform_type C, class T> bool set_uniform_array(const std::string_view name, const T *values, size_t size);

		template <uniform_type C, class T> bool set_uniform(const std::string_view name, const T x) {
			if constexpr(C <= uniform_type::UINT4) {
				const std::array<T, 1> array { x };
				return set_uniform_array<C>(name, array.data(), 1u);
			} else {
				return set_uniform_array<C>(name, x, 1u);
			}
		}

		template <uniform_type C, class T> bool set_uniform(const std::string_view name, T x, T y) {
			const std::array<T, 2> array { x, y };
			return set_uniform_array<C>(name, array.data(), 1u);
		}

		template <uniform_type C, class T> bool set_uniform(const std::string_view name, T x, T y, T z) {
			const std::array<T, 3> array { x, y, z };
			return set_uniform_array<C>(name, array.data(), 1u);
		}

		template <uniform_type C, class T> bool set_uniform(const std::string_view name, T x, T y, T z, T w) {
			const std::array<T, 4> array { x, y, z, w };
			return set_uniform_array<C>(name, array.data(), 1u);
		}

		template <class T> std::optional<T> get_uniform(const std::string_view name) const;

		void use() const { glUseProgram(program_ref); }
	private:
		std::map<std::string, GLuint, std::less<>> uniforms;
		GLuint program_ref;
	};
};


#endif
