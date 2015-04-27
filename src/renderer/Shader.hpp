#pragma once

#include <GL/glew.h>
#include <string>

namespace bey
{

	class Shader
	{		
	private:
		std::string vs_filepath;
		std::string fs_filepath;

	public:

		GLuint program, vertex_shader, fragment_shader;

		//attribute
		GLint posL_attribute;
		GLint color_attribute;
		GLint uv_attribute;
		GLint normal_attribute;		

		Shader();
		~Shader();
		
		GLuint compile_shader(const std::string& filepath, GLint shader_type);
		void load_shader_program(const std::string& vs_filepath, const std::string& fs_filepath);
		void bind() const;
		void unbind() const;
	};

}