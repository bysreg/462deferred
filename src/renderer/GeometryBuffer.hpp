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
			DIFFUSE, // r, g, b : object material's diffuse color * texture color
			NORMAL,
			TEXCOORD,
			SPECULAR, // r, g, b : object material's specular color * texture color || a : specular power
			LIGHT_ACCUMULATION, 
			NUM_TEXTURES,
		};

		enum class BindType
		{
			READ = 0,
			WRITE,
			READ_AND_WRITE,			
		};

		void initialize(int screen_width, int screen_height);
		void bind(BindType bind_type, const Shader* shader = nullptr);
		void unbind(BindType bind_type);
		void set_read_buffer(TextureType texture_type);
		void dump_geometry_buffer(int screen_width, int screen_height);
		const Shader* get_geometry_pass_shader() const;
		void bind_texture(const Shader* shader, const GLchar* uniform_name, GeometryBuffer::TextureType texture_type);

		void bind_light_accum_buffer(GLenum target);
		void unbind_light_accum_buffer(GLenum target);

	private:
		GLuint geometry_buffer_fbo_id;
		GLuint depth_id;	
		GLuint uni_texture_ids[(unsigned int)TextureType::NUM_TEXTURES];
		Shader shader;
		GLuint texture_ids[(unsigned int)TextureType::NUM_TEXTURES];

	};	
}