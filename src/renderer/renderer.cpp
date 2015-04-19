#include "renderer.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <cstddef>

using namespace bey;

bool Renderer::initialize(const Scene& scene, const RendererInitData& data )
{
	glViewport(0, 0, data.screen_width, data.screen_height);
	glClearColor(0.1f, 1.0f, 0.1f, 0.0f);	

	screen_width = data.screen_width;
	screen_height = data.screen_height;

	initialize_buffers();
	initialize_static_models(scene.get_static_models(), scene.num_static_models());
	initialize_shaders();

	return true;
}

void Renderer::initialize_buffers()
{
	glGenTextures(1, &depth_buffer_id);
	glGenTextures(1, &normal_buffer_id);
	glGenTextures(1, &diffuse_buffer_id);

	GLint buffers[] = { normal_buffer_id, diffuse_buffer_id };

	for (int i = 0; i < sizeof(buffers) / sizeof(buffers[0]); i++)
	{
		glBindTexture(GL_TEXTURE_2D, buffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	//allocate depth buffer
	{
		glBindTexture(GL_TEXTURE_2D, depth_buffer_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screen_width, screen_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);		
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Renderer::initialize_shaders()
{
	Shader shader;

	shader.load_shader_program("../../shaders/simple_triangle.vs", "../../shaders/simple_triangle.fs");
	shaders.push_back(shader);
}

void Renderer::initialize_static_models(const StaticModel* static_models, size_t num_static_models)
{	
	RenderData* prev = nullptr;
	for (size_t i = 0; i < num_static_models; i++)
	{
		const StaticModel& static_model = static_models[i];
		for (int j = 0; j < static_model.model->get_mesh_groups_size(); j++)
		{
			const Vertex* vertices = static_model.model->get_vertices(j);
			const unsigned int* indices = static_model.model->get_indices(j);
			size_t vertices_size = static_model.model->num_vertices(j) * sizeof(vertices[0]);
			size_t indices_size = static_model.model->num_indices(j) * sizeof(indices[0]);
			GLuint vertices_id;
			GLuint indices_id;						

			glGenBuffers(1, &vertices_id);
			glGenBuffers(1, &indices_id);

			glBindBuffer(GL_ARRAY_BUFFER, vertices_id);
			glBufferData(GL_ARRAY_BUFFER, vertices_size, &vertices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, &indices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			RenderData* render_data = new RenderData;
			render_data->vertices_id = vertices_id;
			render_data->indices_id = indices_id;
			render_data->model = &static_model;
			render_data->group_id = j;
			render_data->is_dirty = true;
			render_data->world_mat = glm::scale(glm::mat4(), static_model.scale);
			//render_data->world_mat = glm::eulerAngleYXZ(static_model.orientation.y, static_model.orientation.x, static_model.orientation.z) * render_data->world_mat; // somehow, this does not look right ? 
			render_data->world_mat = glm::toMat4(static_model.orientation) * render_data->world_mat;
			render_data->world_mat = glm::translate(glm::mat4(), static_model.position) * render_data->world_mat;
			
			if (head == nullptr)
			{
				head = render_data;
				prev = render_data;
			}
			else
			{
				prev->next = render_data;
				prev = prev->next;
			}
		}		
	}
}

void Renderer::set_uniforms(GLuint shader_program, const RenderData& render_data, const Camera& camera)
{
	GLint uni_world = glGetUniformLocation(shader_program, "world");
	if (uni_world != -1) 
	{
		glUniformMatrix4fv(uni_world, 1, GL_FALSE, glm::value_ptr(render_data.world_mat));
	}

	GLint uni_view = glGetUniformLocation(shader_program, "view");
	if (uni_view != -1) 
	{
		glUniformMatrix4fv(uni_view, 1, GL_FALSE, glm::value_ptr(camera.get_view_matrix()));
	}

	GLint uni_proj = glGetUniformLocation(shader_program, "proj");
	if (uni_proj != -1) 
	{
		glUniformMatrix4fv(uni_proj, 1, GL_FALSE, glm::value_ptr(camera.get_projection_matrix()));
	}

	GLint uni_proj_view = glGetUniformLocation(shader_program, "proj_view");
	if (uni_proj_view != -1)
	{
		glUniformMatrix4fv(uni_proj_view, 1, GL_FALSE, glm::value_ptr(camera.get_projection_matrix() * camera.get_view_matrix()));
	}
}

void Renderer::render( const Camera& camera, const Scene& scene )
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_DEPTH_TEST);	

	const StaticModel* static_models = scene.get_static_models();

	RenderData* render_data = head;
	while (render_data != nullptr)
	{
		Shader& shader = shaders[0];
		shader.bind();		

		const StaticModel& static_model = *(render_data->model);
		size_t vertices_size = static_model.model->num_vertices(render_data->group_id) * sizeof(Vertex);
		size_t indices_size = static_model.model->num_indices(render_data->group_id) * sizeof(unsigned int);		

		if (render_data->is_dirty)
		{
			glBindBuffer(GL_ARRAY_BUFFER, render_data->vertices_id);
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_size, static_model.model->get_vertices(render_data->group_id));				
			render_data->is_dirty = false;
		}
		else
		{
			glBindBuffer(GL_ARRAY_BUFFER, render_data->vertices_id);		
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data->indices_id);

		//set shader's attributes and uniforms
		if (shader.posL_attribute != -1)
		{
			glEnableVertexAttribArray(shader.posL_attribute);
			glVertexAttribPointer(shader.posL_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
		}

		if (shader.color_attribute != -1)
		{
			glEnableVertexAttribArray(shader.color_attribute);
			glVertexAttribPointer(shader.color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, color));
		}

		//mandatory shader variables
		set_uniforms(shader.program, *render_data, camera);

		glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

		render_data = render_data->next;

		shader.unbind();
	}	
	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::release()
{
	
}