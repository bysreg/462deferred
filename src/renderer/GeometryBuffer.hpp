#pragma once

#include "renderer/Shader.hpp"
#include <GL/glew.h>

namespace bey
{
	class GeometryBuffer
	{		
	public:
		GeometryBuffer();
		~GeometryBuffer();

		enum TextureType
		{
			POSITION = 0,
			DIFFUSE,
			NORMAL,
			TEXCOORD,
			SPECULAR,
			LIGHT_ACCUMULATION,
			NUM_TEXTURES,
		};

		enum class BindType
		{
			READ = 0,
			WRITE,
			READ_AND_WRITE,
			TEXTURE,
		};

		void initialize(int screen_width, int screen_height);
		void bind(BindType bind_type, const Shader* shader = nullptr);
		void unbind(BindType bind_type);
		void set_read_buffer(TextureType texture_type);
		void dump_geometry_buffer(int screen_width, int screen_height);
		const Shader* get_geometry_pass_shader() const;
		void bind_texture(const Shader* shader, const GLchar* uniform_name, GeometryBuffer::TextureType texture_type);

	private:
		GLuint fbo_id;
		GLuint depth_id;
		GLuint texture_ids[(unsigned int) TextureType::NUM_TEXTURES];
		GLuint uni_texture_ids[(unsigned int)TextureType::NUM_TEXTURES];
		Shader shader;

	};	
}