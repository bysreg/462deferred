#pragma once

#include <renderer/Shader.hpp>
#include <iostream>

using namespace bey;

static char* read_source(const std::string& filepath)
{
	FILE * pf;
	if (fopen_s(&pf, filepath.c_str(), "rb") != 0)
		return NULL;
	fseek(pf, 0, SEEK_END);
	long size = ftell(pf);
	fseek(pf, 0, SEEK_SET);

	char * shader_source = new char[size + 1];
	fread(shader_source, sizeof(char), size, pf);
	shader_source[size] = 0;
	fclose(pf);

	return shader_source;
}

Shader::Shader()
{
}

Shader::~Shader()
{
	//fixme : delete stuffs here
}

//shader type is either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
GLuint Shader::compile_shader(const std::string& filepath, GLint shader_type)
{
	GLint compile_result;
	GLuint shader = glCreateShader(shader_type);

	const char* source = read_source(filepath);

	glShaderSource(shader, 1, &source, NULL);
	delete[] source;

	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);

	if (!compile_result)
	{
		GLint info_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_length);

		if (info_length > 1)
		{
			char* info_log = new char[info_length];
			glGetShaderInfoLog(shader, info_length, NULL, info_log);
			std::cout << "Error compiling shader " << filepath << " : " << info_log << std::endl;

			delete[] info_log;
		}		

		exit(2);
	}

	return shader;
}

void Shader::load_shader_program(const std::string& vs_filepath, const std::string& fs_filepath)
{
	GLuint compiled_vs_id = compile_shader(vs_filepath, GL_VERTEX_SHADER);
	GLuint compiled_fs_id = compile_shader(fs_filepath, GL_FRAGMENT_SHADER);
	GLint link_status;
	program = glCreateProgram();

	glAttachShader(program, compiled_vs_id);
	glAttachShader(program, compiled_fs_id);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);

	if (link_status == GL_FALSE)
	{
		GLint info_log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

		if (info_log_length > 1)
		{
			char* info_log = new char[sizeof(char) * info_log_length];

			glGetProgramInfoLog(program, info_log_length, NULL, info_log);			
			std::cout << "Error linking program : " << info_log << std::endl;

			delete info_log;			
		}

		exit(2);		
	}

	//load all the attributes
	posL_attribute = glGetAttribLocation(program, "a_posL");
	color_attribute = glGetAttribLocation(program, "a_color");
}

void Shader::bind()
{
	glUseProgram(program);
}