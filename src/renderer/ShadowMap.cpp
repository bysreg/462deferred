#include "renderer/ShadowMap.hpp"
#include "scene/Vertex.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace bey;
static const bool debug = true;

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

	// for debugging, color attachment
	glGenTextures(1, &debug_texture_id);
	glBindTexture(GL_TEXTURE_2D, debug_texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, debug_texture_id, 0);

	//shadow map will only store depth values
	glGenTextures(1, &shadow_texture_id);
	glBindTexture(GL_TEXTURE_2D, shadow_texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_texture_id, 0);

	//for debugging
	GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);
	//this should be the actual draw buffer
	//glDrawBuffer(GL_NONE); // we never write any color to color buffer, so we dont need draw buffer	
	
	//glReadBuffer(GL_NONE);

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
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
	shader_first_pass.bind();

	GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);

	//clear the shadow texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // color bit perhaps does not need to be cleared ? 
}

void ShadowMap::unbind_first_pass()
{
	shader_first_pass.unbind();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void ShadowMap::set_attribute_first_pass()
{
	if (shader_first_pass.posL_attribute != -1)
	{
		glEnableVertexAttribArray(shader_first_pass.posL_attribute);
		glVertexAttribPointer(shader_first_pass.posL_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
	}
}

void ShadowMap::set_matrix_first_pass(const glm::mat4& matrix)
{
	GLint uni_world = glGetUniformLocation(shader_first_pass.program, "u_proj_view_world");
	if (uni_world != -1)
	{
		glUniformMatrix4fv(uni_world, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}

void ShadowMap::dump_shadow_texture(int screen_width, int screen_height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glReadBuffer(GL_COLOR_ATTACHMENT0 + 0);
	glBlitFramebuffer(0, 0, screen_width, screen_height, 0, 0, screen_width, screen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

const Shader& ShadowMap::get_first_pass_shader() const
{
	return shader_first_pass;
}