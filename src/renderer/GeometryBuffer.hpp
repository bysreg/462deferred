#pragma once

#include "renderer/Shader.hpp"
#include <GL/glew.h>

namespace bey
{
	class GeometryBuffer
	{
	public:
		static const unsigned int NUM_TEXTURES = 4;

	private:
		GLuint fbo_id;
		GLuint depth_id;
		GLuint texture_ids[NUM_TEXTURES];
		Shader shader;

	public:
		GeometryBuffer();
		~GeometryBuffer();

		enum class TextureType
		{
			POSITION = 0, 
			DIFFUSE, 
			NORMAL, 
			TEXCOORD			
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
	};
}