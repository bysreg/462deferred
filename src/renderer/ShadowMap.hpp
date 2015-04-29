#pragma once

#include "renderer/Shader.hpp"
#include <GL/glew.h>

namespace bey
{
	class ShadowMap
	{
	public :
		ShadowMap();
		~ShadowMap();

		void initialize(int screen_width, int screen_height);
		void bind();
		void unbind();
	private:
		GLuint fbo_id;
		Shader shader;
		GLuint shadow_texture_id;
	};
}