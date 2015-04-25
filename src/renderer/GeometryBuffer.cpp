#include "renderer/GeometryBuffer.hpp"
#include <iostream>

using namespace bey;

GeometryBuffer::GeometryBuffer()
{
}

GeometryBuffer::~GeometryBuffer()
{
}

void GeometryBuffer::initialize(int screen_width, int screen_height)
{
	shader.load_shader_program("../../shaders/geometry_pass.vs", "../../shaders/geometry_pass.fs");

	// Create the FBO
	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);

	// Create the gbuffer textures
	glGenTextures(NUM_TEXTURES, texture_ids);
	glGenTextures(1, &depth_id);

	for (unsigned int i = 0; i < NUM_TEXTURES; i++)
	{
		glBindTexture(GL_TEXTURE_2D, texture_ids[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture_ids[i], 0);
	}

	// depth
	glBindTexture(GL_TEXTURE_2D, depth_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_id, 0);

	GLenum draw_buffers[NUM_TEXTURES];
	for (int i = 0; i < NUM_TEXTURES; i++)
	{
		draw_buffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	glDrawBuffers(NUM_TEXTURES, draw_buffers);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "FB error, status: 0x" << status << std::endl;
		exit(EXIT_FAILURE);
		return;
	}

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void GeometryBuffer::bind(BindType bind_type)
{
	if (bind_type == BindType::READ)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id);
	}
	else if (bind_type == BindType::WRITE)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
		//set the shaders
		shader.bind();
	}
}

void GeometryBuffer::unbind(BindType bind_type)
{
	if (bind_type == BindType::READ)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	}
	else if (bind_type == BindType::WRITE)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		//unset the shaders
		shader.unbind();
	}
}

void GeometryBuffer::set_read_buffer(TextureType texture_type)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + (int)texture_type);
}

void GeometryBuffer::dump_geometry_buffer(int screen_width, int screen_height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	bind(BindType::READ);

	GLsizei half_width = (GLsizei)(screen_width / 2.0f);
	GLsizei half_height = (GLsizei)(screen_height / 2.0f);

	set_read_buffer(TextureType::POSITION);
	glBlitFramebuffer(0, 0, screen_width, screen_height, 0, 0, half_width, half_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	set_read_buffer(TextureType::DIFFUSE);
	glBlitFramebuffer(0, 0, screen_width, screen_height, 0, half_height, half_width, screen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	set_read_buffer(TextureType::NORMAL);
	glBlitFramebuffer(0, 0, screen_width, screen_height, half_width, half_height, screen_width, screen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	set_read_buffer(TextureType::TEXCOORD);
	glBlitFramebuffer(0, 0, screen_width, screen_height, half_width, 0, screen_width, half_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

const Shader* GeometryBuffer::get_geometry_pass_shader() const
{
	return &shader;
}