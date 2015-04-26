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
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);	

	screen_width = data.screen_width;
	screen_height = data.screen_height;
	head = nullptr;
	
	initialize_static_models(scene.get_static_models(), scene.num_static_models());
	geometry_buffer.initialize(screen_width, screen_height);
	initialize_shaders();
	initialize_primitives();

	return true;
}

void Renderer::initialize_primitives()
{
	quad = create_quad();
}

void Renderer::initialize_shaders()
{
	Shader shader;
	shader.load_shader_program("../../shaders/simple_triangle.vs", "../../shaders/simple_triangle.fs");
	shaders.push_back(shader);

	directional_light_shader.load_shader_program("../../shaders/directional_light_pass.vs", "../../shaders/directional_light_pass.fs");
}

void Renderer::initialize_material(const StaticModel& static_model, int group_index, RenderData& render_data)
{
	const ObjModel::ObjMtl* material = static_model.model->get_material(group_index);
	
	//diffuse
	std::unordered_map<std::string, GLuint>::const_iterator got = texture_ids.find(material->map_Kd_path);
	GLuint diffuse_texture_id;
	if (got == texture_ids.end())
	{
		const sf::Uint8* diffuse_texture_pixel_pointer = static_model.model->get_texture(material->map_Kd)->getPixelsPtr();
		sf::Vector2u diffuse_texture_size = static_model.model->get_texture(material->map_Kd)->getSize();		
		glGenTextures(1, &diffuse_texture_id);
		glBindTexture(GL_TEXTURE_2D, diffuse_texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, diffuse_texture_size.x, diffuse_texture_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, diffuse_texture_pixel_pointer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		texture_ids[material->map_Kd_path] = diffuse_texture_id;
	}
	else
	{
		diffuse_texture_id = got->second;
	}

	render_data.diffuse_texture_id = diffuse_texture_id;
}

void Renderer::initialize_static_models(const StaticModel* static_models, size_t num_static_models)
{	
	RenderData* prev = nullptr;
	for (size_t i = 0; i < num_static_models; i++)
	{
		const StaticModel& static_model = static_models[i];
		const Vertex* vertices = static_model.model->get_vertices();
		size_t vertices_size = static_model.model->num_vertices() * sizeof(vertices[0]);
		GLuint vertices_id;

		glGenBuffers(1, &vertices_id);
		glBindBuffer(GL_ARRAY_BUFFER, vertices_id);
		glBufferData(GL_ARRAY_BUFFER, vertices_size, &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		for (int j = 0; j < static_model.model->get_mesh_groups_size(); j++)
		{			
			const unsigned int* indices = static_model.model->get_indices(j);			
			size_t indices_size = static_model.model->num_indices(j) * sizeof(indices[0]);			
			GLuint indices_id;						
		
			glGenBuffers(1, &indices_id);			

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, &indices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			RenderData* render_data = new RenderData;
			render_data->vertices_id = vertices_id;
			render_data->indices_id = indices_id;
			render_data->model = &static_model;			
			render_data->group_id = j;
			render_data->material = static_model.model->get_material(j);
			render_data->is_dirty = true;
			render_data->world_mat = glm::scale(glm::mat4(), static_model.scale);			
			render_data->world_mat = glm::toMat4(static_model.orientation) * render_data->world_mat;
			render_data->world_mat = glm::translate(glm::mat4(), static_model.position) * render_data->world_mat;
			
			initialize_material(static_model, j, *render_data);

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

RenderData* Renderer::create_quad()
{
	RenderData* rd = new RenderData;
	
	Vertex vertices[4];
	vertices[0].position = glm::vec3(-1, 1, 0); // top left
	vertices[1].position = glm::vec3(-1, -1, 0); // bottom left
	vertices[2].position = glm::vec3(1, 1, 0); // top right
	vertices[3].position = glm::vec3(1, -1, 0); // bottom right

	size_t vertices_size = 4 * sizeof(vertices[0]);
	GLuint vertices_id;

	glGenBuffers(1, &vertices_id);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_id);
	glBufferData(GL_ARRAY_BUFFER, vertices_size, &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	rd->vertices_id = vertices_id;
	rd->world_mat = glm::mat4(); // identity

	return rd;
}

void Renderer::set_attributes(const Shader& shader)
{
	if (shader.posL_attribute != -1)
	{
		glEnableVertexAttribArray(shader.posL_attribute);
		glVertexAttribPointer(shader.posL_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
	}

	if (shader.color_attribute != -1)
	{
		glEnableVertexAttribArray(shader.color_attribute);
		glVertexAttribPointer(shader.color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));
	}

	if (shader.uv_attribute != -1)
	{
		glEnableVertexAttribArray(shader.uv_attribute);
		glVertexAttribPointer(shader.uv_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, tex_coord));
	}

	if (shader.normal_attribute != -1)
	{
		glEnableVertexAttribArray(shader.normal_attribute);
		glVertexAttribPointer(shader.normal_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
	}
}

void Renderer::set_uniforms(GLuint shader_program, const RenderData& render_data, const Camera& camera)
{
	GLint uni_world = glGetUniformLocation(shader_program, "u_world");
	if (uni_world != -1) 
	{
		glUniformMatrix4fv(uni_world, 1, GL_FALSE, glm::value_ptr(render_data.world_mat));
	}

	GLint uni_view = glGetUniformLocation(shader_program, "u_view");
	if (uni_view != -1) 
	{
		glUniformMatrix4fv(uni_view, 1, GL_FALSE, glm::value_ptr(camera.get_view_matrix()));
	}

	GLint uni_proj = glGetUniformLocation(shader_program, "u_proj");
	if (uni_proj != -1) 
	{
		glUniformMatrix4fv(uni_proj, 1, GL_FALSE, glm::value_ptr(camera.get_projection_matrix()));
	}

	GLint uni_proj_view = glGetUniformLocation(shader_program, "u_proj_view");
	if (uni_proj_view != -1)
	{
		glUniformMatrix4fv(uni_proj_view, 1, GL_FALSE, glm::value_ptr(camera.get_projection_matrix() * camera.get_view_matrix()));
	}

	GLint uni_diffuse_texture = glGetUniformLocation(shader_program, "u_diffuse_texture");
	if (uni_diffuse_texture != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, render_data.diffuse_texture_id);
		glUniform1i(uni_diffuse_texture, 0);
	}

	GLint uni_screen_size = glGetUniformLocation(shader_program, "u_screen_size");
	if (uni_screen_size != -1)
	{
		glUniform2f(uni_screen_size, screen_width, screen_height);
	}

	//material based uniform
	const ObjModel::ObjMtl* material = render_data.material;

	GLint uni_specular_power = glGetUniformLocation(shader_program, "u_specular_power");
	if (uni_specular_power != -1)
	{
		glUniform1f(uni_specular_power, material->Ns);
	}

	GLint uni_ambient = glGetUniformLocation(shader_program, "u_ambient");
	if (uni_ambient != -1)
	{
		glUniform3f(uni_ambient, material->Ka.r, material->Ka.g, material->Ka.b);
	}

	GLint uni_diffuse = glGetUniformLocation(shader_program, "u_diffuse");
	if (uni_diffuse != -1)
	{
		glUniform3f(uni_diffuse, material->Kd.r, material->Kd.g, material->Kd.b);
	}

	GLint uni_specular = glGetUniformLocation(shader_program, "u_specular");
	if (uni_specular != -1)
	{
		glUniform3f(uni_specular, material->Ks.r, material->Ks.g, material->Ks.b);
	}
}

void Renderer::geometry_pass(const Scene& scene)
{
	geometry_buffer.bind(GeometryBuffer::BindType::WRITE);
	const Shader& shader = *geometry_buffer.get_geometry_pass_shader();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	RenderData* render_data = head;
	while (render_data != nullptr)
	{	
		const StaticModel& static_model = *(render_data->model);
		const ObjModel::MeshGroup* mesh_group = render_data->model->model->get_mesh_group(render_data->group_id);
		const ObjModel::ObjMtl* material = (render_data->model)->model->get_material(render_data->group_id);
		size_t indices_size = static_model.model->num_indices(render_data->group_id) * sizeof(unsigned int);

		glBindBuffer(GL_ARRAY_BUFFER, render_data->vertices_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data->indices_id);

		//set shader's attributes and uniforms		
		set_attributes(shader);
		set_uniforms(shader.program, *render_data, scene.camera);

		glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

		render_data = render_data->next;
	}
	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	geometry_buffer.unbind(GeometryBuffer::BindType::WRITE);

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
}

void Renderer::render( const Camera& camera, const Scene& scene )
{
	geometry_pass(scene);

	directional_light_pass(scene);
	geometry_buffer.dump_geometry_buffer(screen_width, screen_height);	
}

void Renderer::begin_light_pass(const Scene& scene)
{
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	geometry_buffer.bind(GeometryBuffer::BindType::READ);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::directional_light_pass(const Scene& scene)
{
	//render with quad (all pixels in the screen will be affected by sunlight)
	RenderData* render_data = quad;
	const DirectionalLight& sunlight = scene.get_sunlight();

	geometry_buffer.bind(GeometryBuffer::BindType::READ_AND_WRITE);
	geometry_buffer.bind(GeometryBuffer::BindType::TEXTURE, &directional_light_shader);

	directional_light_shader.bind();		

	//set directional light's uniform
	GLuint uni_light_direction = glGetUniformLocation(directional_light_shader.program, "u_light_direction"); // assume it exists
	if (uni_light_direction != -1)
	{
		glUniform3f(uni_light_direction, sunlight.direction.x, sunlight.direction.y, sunlight.direction.z);
	}

	GLuint uni_light_color = glGetUniformLocation(directional_light_shader.program, "u_light_color");
	if (uni_light_color != -1)
	{
		glUniform3f(uni_light_color, sunlight.color.r, sunlight.color.g, sunlight.color.b);
	}

	glBindBuffer(GL_ARRAY_BUFFER, render_data->vertices_id);

	//set shader's attributes and uniforms		
	set_attributes(directional_light_shader);
	set_uniforms(directional_light_shader.program, *render_data, scene.camera);

	GLint uni_diffuse_texture = glGetUniformLocation(directional_light_shader.program, "u_g_position");
	if (uni_diffuse_texture != -1)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 3);
		glUniform1i(uni_diffuse_texture, 2);
	}

	//geometry_buffer.bind_texture(&directional_light_shader, "u_g_position", GeometryBuffer::TextureType::POSITION);
	//geometry_buffer.bind_texture(&directional_light_shader, "u_g_specular", GeometryBuffer::TextureType::SPECULAR);
	//geometry_buffer.bind_texture(&directional_light_shader, "u_g_diffuse", GeometryBuffer::TextureType::DIFFUSE);
	//geometry_buffer.bind_texture(&directional_light_shader, "u_g_normal", GeometryBuffer::TextureType::NORMAL);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	directional_light_shader.unbind();

	geometry_buffer.unbind(GeometryBuffer::BindType::TEXTURE);
	geometry_buffer.unbind(GeometryBuffer::BindType::READ_AND_WRITE);
}

void Renderer::release()
{
	
}