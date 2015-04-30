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

		Shader shader_first_pass; // fixme

		void initialize(int screen_width, int screen_height);
		void bind_first_pass();
		void unbind_first_pass();		

		void set_attribute_first_pass();
		void set_matrix_first_pass(const glm::mat4& matrix);
		void dump_shadow_texture(int screen_width, int screen_height);
	private:
		GLuint fbo_id;
		GLuint debug_texture_id;
		
		GLuint shadow_texture_id;
	};
}