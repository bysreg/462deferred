#pragma once

#include "renderer/Shader.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace bey
{
	class ShadowMap
	{
	public :
		ShadowMap();
		~ShadowMap();

		void initialize(int screen_width, int screen_height);
		void bind_first_pass();
		void unbind_first_pass();		
		void set_matrix_first_pass(const glm::mat4& matrix);
	private:
		GLuint fbo_id;
		Shader shader_first_pass;
		GLuint shadow_texture_id;
	};
}