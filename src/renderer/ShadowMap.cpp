#include "renderer/ShadowMap.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace bey;

ShadowMap::ShadowMap()
{
}

ShadowMap::~ShadowMap()
{
}

void ShadowMap::initialize(int screen_width, int screen_height)
{
	shader_first_pass.load_shader_program("../../shaders/shadow_first_pass.vs", "../../shaders/shadow_first_pass.fs");

	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);

	//shadow map will only store depth values
	glGenTextures(1, &shadow_texture_id);
	glBindTexture(GL_TEXTURE_2D, shadow_texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_texture_id, 0);

	glDrawBuffer(GL_NONE); // we never write any color to color buffer, so we dont need draw buffer
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", status);
		exit(EXIT_FAILURE);
	}

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void ShadowMap::bind_first_pass()
{
	shader_first_pass.bind();
}

void ShadowMap::unbind_first_pass()
{
	shader_first_pass.unbind();
}

void ShadowMap::set_matrix_first_pass(const glm::mat4& matrix)
{
	GLint uni_world = glGetUniformLocation(shader_first_pass.program, "u_proj_view_world");
	if (uni_world != -1)
	{
		glUniformMatrix4fv(uni_world, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}