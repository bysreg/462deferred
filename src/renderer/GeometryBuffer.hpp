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
			LIGHT_ACCUMULATION,
			NUM_TEXTURES,
		};

		enum class BindType
		{
			READ = 0,
			WRITE,
		};

		void initialize(int screen_width, int screen_height);
		void bind(BindType bind_type);
		void unbind(BindType bind_type);
		void set_read_buffer(TextureType texture_type);
		void dump_geometry_buffer(int screen_width, int screen_height);
		const Shader* get_geometry_pass_shader() const;

	private:
		GLuint fbo_id;
		GLuint depth_id;
		GLuint texture_ids[(unsigned int) TextureType::NUM_TEXTURES];
		Shader shader;
	};	
}