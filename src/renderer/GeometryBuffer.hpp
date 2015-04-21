#pragma once

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

	public:
		GeometryBuffer();
		~GeometryBuffer();

		enum class TextureType
		{
			POSITION, 
			DIFFUSE, 
			NORMAL, 
			TEXCOORD			
		};

		void initialize(int screen_width, int screen_height);
	};
}