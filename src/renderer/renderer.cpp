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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	

	screen_width = data.screen_width;
	screen_height = data.screen_height;
	head = nullptr;
	
	initialize_static_models(scene.get_static_models(), scene.num_static_models());
	geometry_buffer.initialize(screen_width, screen_height);
	shadow_map.initialize(screen_width, screen_height);
	initialize_shaders();
	initialize_primitives();

	return true;
}

void Renderer::initialize_primitives()
{
	cone = create_cone(); // somehow when creating cone after sphere, there will be some kind of artifacts in AMD
	quad = create_quad();
	sphere = create_sphere();
}

void Renderer::initialize_shaders()
{
	Shader shader;
	shader.load_shader_program("../../shaders/simple_triangle.vs", "../../shaders/simple_triangle.fs");
	shaders.push_back(shader);

	Shader test_shader;
	test_shader.load_shader_program("../../shaders/shadow_first_pass.vs", "../../shaders/shadow_first_pass.fs");
	shaders.push_back(test_shader);

	directional_light_shader.load_shader_program("../../shaders/directional_light_pass.vs", "../../shaders/directional_light_pass.fs");
	point_light_shader.load_shader_program("../../shaders/point_light_pass.vs", "../../shaders/point_light_pass.fs");
	stencil_shader.load_shader_program("../../shaders/stencil_pass.vs", "../../shaders/stencil_pass.fs");
	spot_light_shader.load_shader_program("../../shaders/spot_light_pass.vs", "../../shaders/spot_light_pass.fs");
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		texture_ids[material->map_Kd_path] = diffuse_texture_id;
	}
	else
	{
		diffuse_texture_id = got->second;
	}

	render_data.diffuse_texture_id = diffuse_texture_id;
}

void calc_bounding_box(BoundingBox& bounding_box, const glm::mat4& world, const StaticModel& static_model)
{
	//get the bounding box in local position
	BoundingBox local_bb = static_model.get_bounding_box();
	float local_x[2] = {local_bb.min.x, local_bb.max.x};
	float local_y[2] = {local_bb.min.y, local_bb.max.y};
	float local_z[2] = { local_bb.min.z, local_bb.max.z};
	bounding_box.min = glm::vec3(world * glm::vec4(local_bb.min, 1));

	for (int i = 0; i < 2; i++)
	{
		float cur_local_x = local_x[i];
		for (int j = 0; j < 2; j++)
		{
			float cur_local_y = local_y[j];
			for (int k = 0; k < 2; k++)
			{
				float cur_local_z = local_z[k];
				glm::vec3 world_pos = glm::vec3(world * glm::vec4(cur_local_x, cur_local_y, cur_local_z, 1));
				
				bounding_box.min.x = std::min(bounding_box.min.x, world_pos.x);
				bounding_box.min.y = std::min(bounding_box.min.y, world_pos.y);
				bounding_box.min.z = std::min(bounding_box.min.z, world_pos.z);

				bounding_box.max.x = std::max(bounding_box.max.x, world_pos.x);
				bounding_box.max.y = std::max(bounding_box.max.y, world_pos.y);
				bounding_box.max.z = std::max(bounding_box.max.z, world_pos.z);
			}
		}
	}
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

RenderData* Renderer::create_sphere()
{
	RenderData* render_data = new RenderData;	

	StaticModel* static_model = new StaticModel;
	ObjModel* model = new ObjModel;
	if (!model->loadFromFile("../../scenes/", "models/sphere.obj"))
	{
		if (!model->loadFromFile("scenes/", "models/sphere.obj"))
		{
			std::cerr << "Error reading sphere object file" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	
	static_model->model = model;

	const Vertex* vertices = model->get_vertices();
	size_t vertices_size = model->num_vertices() * sizeof(vertices[0]);
	GLuint vertices_id;

	glGenBuffers(1, &vertices_id);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_id);
	glBufferData(GL_ARRAY_BUFFER, vertices_size, &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	const unsigned int* indices = model->get_indices(0);
	size_t indices_size = model->num_indices(0) * sizeof(indices[0]);
	GLuint indices_id;

	glGenBuffers(1, &indices_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, &indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	render_data->vertices_id = vertices_id;
	render_data->indices_id = indices_id;
	render_data->world_mat = glm::mat4();
	render_data->model = static_model;	
	render_data->group_id = 0;

	return render_data;
}

RenderData* Renderer::create_cone()
{
	RenderData* render_data = new RenderData;

	StaticModel* static_model = new StaticModel;
	ObjModel* model = new ObjModel;
	if (!model->loadFromFile("../../scenes/", "models/cone_hilman.obj"))
	{
		if (!model->loadFromFile("scenes/", "models/cone_hilman.obj"))
		{
			std::cerr << "Error reading cone object file" << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	static_model->model = model;

	const Vertex* vertices = model->get_vertices();
	size_t vertices_size = model->num_vertices() * sizeof(vertices[0]);
	GLuint vertices_id;

	glGenBuffers(1, &vertices_id);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_id);
	glBufferData(GL_ARRAY_BUFFER, vertices_size, &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	const unsigned int* indices = model->get_indices(0);
	size_t indices_size = model->num_indices(0) * sizeof(indices[0]);
	GLuint indices_id;

	glGenBuffers(1, &indices_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, &indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	render_data->vertices_id = vertices_id;
	render_data->indices_id = indices_id;
	render_data->world_mat = glm::mat4();
	render_data->model = static_model;
	render_data->group_id = 0;

	return render_data;
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

	GLint uni_proj_view_world = glGetUniformLocation(shader_program, "u_proj_view_world");
	if (uni_proj_view_world != -1)
	{
		glUniformMatrix4fv(uni_proj_view_world, 1, GL_FALSE, glm::value_ptr(camera.get_projection_matrix() * camera.get_view_matrix() * render_data.world_mat));
	}

	GLint uni_cam_pos = glGetUniformLocation(shader_program, "u_cam_pos");
	if (uni_cam_pos != -1)
	{
		glUniform3f(uni_cam_pos, camera.get_position().x, camera.get_position().y, camera.get_position().z);
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
	
	glDisable(GL_DEPTH_TEST);
}

void Renderer::render( const Camera& camera, const Scene& scene )
{
	geometry_pass(scene);

	begin_light_pass(scene);
	directional_light_pass(scene);	

	size_t num_point_lights = scene.num_point_lights();
	const PointLight* point_lights = scene.get_point_lights();
	for (int i = 0; i < num_point_lights; i++)
	{
		const PointLight& point_light = point_lights[i];

		//adjust the sphere for point light
		sphere->world_mat = glm::scale(glm::mat4(), glm::vec3(point_light.cutoff, point_light.cutoff, point_light.cutoff));
		sphere->world_mat = glm::translate(glm::mat4(), point_light.position) * sphere->world_mat;

		stencil_pass(scene, *sphere);
		point_light_pass(scene, point_light);
	}

	size_t num_spot_lights = scene.num_spot_lights();
	const SpotLight* spot_lights = scene.get_spot_lights();
	for (int i = 0; i < num_spot_lights; i++)
	{
		const SpotLight& spot_light = spot_lights[i];

		//adjust the cone for spot light
		cone->world_mat = glm::scale(glm::mat4(), glm::vec3(spot_light.base_radius, spot_light.base_radius, spot_light.cutoff));
		cone->world_mat = glm::toMat4(spot_light.orientation) * cone->world_mat;
		cone->world_mat = glm::translate(glm::mat4(), spot_light.position) * cone->world_mat;

		stencil_pass(scene, *cone);
		spot_light_pass(scene, spot_light);
	}

	end_light_pass(scene);

	//shadow mapping	
	//shadow_map.bind_first_pass();
	////directional_light_shadow_pass(scene);

	//for (int i = 0; i < num_spot_lights; i++)
	//{
	//	const SpotLight& spot_light = spot_lights[i];

	//	spot_light_shadow_pass(scene, spot_light);
	//}

	//shadow_map.unbind_first_pass();	

	//geometry_buffer.dump_geometry_buffer(screen_width, screen_height);

	//fixme
	//shadow_map.dump_shadow_texture(screen_width, screen_height);
	
	//render_all_models(camera, scene); // for debug

	glDepthMask(GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	for (int i = 0; i < num_spot_lights; i++)
	{
		const SpotLight& spot_light = spot_lights[i];

		glm::vec3 direction = spot_light.orientation * glm::vec3(0, 0, -1);
		glm::vec3 up = spot_light.orientation * glm::vec3(0, 1, 0);
		glm::mat4 light_view_mat = glm::lookAt(spot_light.position, spot_light.position + direction, up);
		//glm::mat4 light_proj_view_mat = scene.camera.get_projection_matrix() * light_view_mat;
		glm::mat4 light_proj_view_mat = scene.camera.get_projection_matrix() * scene.camera.get_view_matrix();

		RenderData* render_data = head;
		while (render_data != nullptr)
		{
			const Shader& shadow_shader = shaders[1];
			shadow_shader.bind();

			const StaticModel& static_model = *(render_data->model);
			const ObjModel::MeshGroup* mesh_group = render_data->model->model->get_mesh_group(render_data->group_id);
			size_t indices_size = static_model.model->num_indices(render_data->group_id) * sizeof(unsigned int);

			glBindBuffer(GL_ARRAY_BUFFER, render_data->vertices_id);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data->indices_id);

			//set shader's attributes and uniforms					
			if (shadow_shader.posL_attribute != -1)
			{
				glEnableVertexAttribArray(shadow_shader.posL_attribute);
				glVertexAttribPointer(shadow_shader.posL_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
			}

			GLint uni_proj_view_world = glGetUniformLocation(shadow_shader.program, "u_proj_view_world");
			if (uni_proj_view_world != -1)
			{
				glUniformMatrix4fv(uni_proj_view_world, 1, GL_FALSE, glm::value_ptr(light_proj_view_mat * render_data->world_mat));
			}

			glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

			//unbind all previous binding
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			shadow_shader.unbind();

			render_data = render_data->next;
		}
		//unbind all previous binding
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}	
}

void Renderer::render_all_models(const Camera& camera, const Scene& scene)
{
	glDepthMask(GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);	
	glDisable(GL_STENCIL_TEST);	

	RenderData* render_data = head;
	while (render_data != nullptr)
	{
		render_model(scene.camera, scene, *render_data, shaders[0]);
		render_data = render_data->next;
	}
	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::render_model(const Camera& camera, const Scene& scene, const RenderData& render_data, const Shader& shader)
{
	shader.bind();

	const StaticModel& static_model = *(render_data.model);
	const ObjModel::MeshGroup* mesh_group = render_data.model->model->get_mesh_group(render_data.group_id);
	size_t indices_size = static_model.model->num_indices(render_data.group_id) * sizeof(unsigned int);

	glBindBuffer(GL_ARRAY_BUFFER, render_data.vertices_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data.indices_id);

	//set shader's attributes and uniforms		
	set_attributes(shader);
	set_uniforms(shader.program, render_data, scene.camera);

	glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	shader.unbind();
}

void Renderer::begin_light_pass(const Scene& scene)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE); // disable writing to depth. we only enable writing to depth on geometry pass. light passes will never write to depth buffer

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	geometry_buffer.bind(GeometryBuffer::BindType::READ_AND_WRITE);
}

void Renderer::end_light_pass(const Scene& scene)
{
	geometry_buffer.unbind(GeometryBuffer::BindType::READ_AND_WRITE);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE); // enable back depth mask writing
}

void Renderer::directional_light_pass(const Scene& scene)
{
	//render with quad (all pixels in the screen will be affected by sunlight)
	RenderData* render_data = quad;
	const DirectionalLight& sunlight = scene.get_sunlight();	

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

	//bind geometry buffers to be sampled
	geometry_buffer.bind_texture(&directional_light_shader, "u_g_position", GeometryBuffer::TextureType::POSITION);
	geometry_buffer.bind_texture(&directional_light_shader, "u_g_specular", GeometryBuffer::TextureType::SPECULAR);
	geometry_buffer.bind_texture(&directional_light_shader, "u_g_diffuse", GeometryBuffer::TextureType::DIFFUSE);
	geometry_buffer.bind_texture(&directional_light_shader, "u_g_normal", GeometryBuffer::TextureType::NORMAL);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	directional_light_shader.unbind();	
}

void Renderer::directional_light_shadow_pass(const Scene& scene)
{
	const DirectionalLight& sunlight = scene.get_sunlight();

	//render all the scene from the light point of view
	//shadow_map
	
	//get all the bounding box of the scene
	size_t num_models = scene.num_static_models();
	const StaticModel* static_models = scene.get_static_models();
	glm::vec3 min;
	glm::vec3 max;

	// get the global bounding box
}

void Renderer::stencil_pass(const Scene& scene, const RenderData& render_data)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE); // stencil pass needs to check both face in order to work properly
	stencil_shader.bind();
	glDrawBuffer(GL_NONE); // disable draw buffer for stencil pass, we dont wanna output black color to the color buffer. this will disable the draw buffer set up by the geometry buffer.

	//stencil preparation
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 0, 0); // always success
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	size_t indices_size = render_data.model->model->num_indices(sphere->group_id) * sizeof(unsigned int);

	//bind vertices and indices
	glBindBuffer(GL_ARRAY_BUFFER, render_data.vertices_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data.indices_id);

	set_attributes(stencil_shader);
	set_uniforms(stencil_shader.program, render_data, scene.camera);

	glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	//bring back geometry draw buffer
	geometry_buffer.setup_draw_buffers();
	stencil_shader.unbind();
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
}

void Renderer::point_light_pass(const Scene& scene, const PointLight& point_light)
{
	glDisable(GL_DEPTH_TEST);	
	
	glEnable(GL_STENCIL_TEST);	
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT); // if we are inside the light volume, if we cull back face, then we cant see the light

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	point_light_shader.bind();

	size_t indices_size = sphere->model->model->num_indices(sphere->group_id) * sizeof(unsigned int);

	//world mat is already calculated outside of this function
	//sphere->world_mat = glm::scale(glm::mat4(), glm::vec3(point_light.cutoff, point_light.cutoff, point_light.cutoff));
	//sphere->world_mat = glm::translate(glm::mat4(), point_light.position) * sphere->world_mat;

	//bind vertices and indices
	glBindBuffer(GL_ARRAY_BUFFER, sphere->vertices_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->indices_id);

	//set shader's attributes and uniforms		
	set_attributes(point_light_shader);
	set_uniforms(point_light_shader.program, *sphere, scene.camera);

	//set light specific properties
	GLuint uni_light_color = glGetUniformLocation(point_light_shader.program, "u_light_color");
	if (uni_light_color != -1)
	{
		glUniform3f(uni_light_color, point_light.color.r, point_light.color.g, point_light.color.b);
	}

	GLuint uni_light_position = glGetUniformLocation(point_light_shader.program, "u_light_position");
	if (uni_light_position != -1)
	{
		glUniform3f(uni_light_position, point_light.position.x, point_light.position.y, point_light.position.z);
	}		

	GLuint uni_light_const_att = glGetUniformLocation(point_light_shader.program, "u_light_const_attenuation");
	if (uni_light_const_att != -1)
	{
		glUniform1f(uni_light_const_att, point_light.Kc);
	}

	GLuint uni_light_linear_att = glGetUniformLocation(point_light_shader.program, "u_light_linear_attenuation");
	if (uni_light_linear_att != -1)
	{
		glUniform1f(uni_light_linear_att, point_light.Kl);
	}

	GLuint uni_light_quad_att = glGetUniformLocation(point_light_shader.program, "u_light_quadratic_attenuation");
	if (uni_light_quad_att != -1)
	{
		glUniform1f(uni_light_quad_att, point_light.Kq);
	}

	//bind geometry buffers to be sampled
	geometry_buffer.bind_texture(&point_light_shader, "u_g_position", GeometryBuffer::TextureType::POSITION);
	geometry_buffer.bind_texture(&point_light_shader, "u_g_specular", GeometryBuffer::TextureType::SPECULAR);
	geometry_buffer.bind_texture(&point_light_shader, "u_g_diffuse", GeometryBuffer::TextureType::DIFFUSE);
	geometry_buffer.bind_texture(&point_light_shader, "u_g_normal", GeometryBuffer::TextureType::NORMAL);

	glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	point_light_shader.unbind();	

	glCullFace(GL_BACK);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::spot_light_pass(const Scene& scene, const SpotLight& spot_light)
{
	glDisable(GL_DEPTH_TEST);
	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT); // if we are inside the light volume, if we cull back face, then we cant see the light

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	spot_light_shader.bind();

	size_t indices_size = cone->model->model->num_indices(cone->group_id) * sizeof(unsigned int);

	//cone calculation is already calculated outside this function
	//cone->world_mat = glm::scale(glm::mat4(), glm::vec3(spot_light.base_radius, spot_light.base_radius, spot_light.cutoff));
	//cone->world_mat = glm::toMat4(spot_light.orientation) * cone->world_mat;
	//cone->world_mat = glm::translate(glm::mat4(), spot_light.position) * cone->world_mat;

	//bind vertices and indices
	glBindBuffer(GL_ARRAY_BUFFER, cone->vertices_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cone->indices_id);

	//set shader's attributes and uniforms		
	set_attributes(spot_light_shader);
	set_uniforms(spot_light_shader.program, *cone, scene.camera);

	//set light specific properties
	GLuint uni_light_color = glGetUniformLocation(spot_light_shader.program, "u_light_color");
	if (uni_light_color != -1)
	{
		glUniform3f(uni_light_color, spot_light.color.r, spot_light.color.g, spot_light.color.b);
	}

	GLuint uni_light_position = glGetUniformLocation(spot_light_shader.program, "u_light_position");
	if (uni_light_position != -1)
	{
		glUniform3f(uni_light_position, spot_light.position.x, spot_light.position.y, spot_light.position.z);
	}

	GLuint uni_light_const_att = glGetUniformLocation(spot_light_shader.program, "u_light_const_attenuation");
	if (uni_light_const_att != -1)
	{
		glUniform1f(uni_light_const_att, spot_light.Kc);
	}

	GLuint uni_light_linear_att = glGetUniformLocation(spot_light_shader.program, "u_light_linear_attenuation");
	if (uni_light_linear_att != -1)
	{
		glUniform1f(uni_light_linear_att, spot_light.Kl);
	}

	GLuint uni_light_quad_att = glGetUniformLocation(spot_light_shader.program, "u_light_quadratic_attenuation");
	if (uni_light_quad_att != -1)
	{
		glUniform1f(uni_light_quad_att, spot_light.Kq);
	}

	GLuint uni_light_correction = glGetUniformLocation(spot_light_shader.program, "u_light_correction_factor");
	if (uni_light_correction != -1)
	{
		glUniform1f(uni_light_correction, spot_light.correction);
	}

	//bind geometry buffers to be sampled
	geometry_buffer.bind_texture(&spot_light_shader, "u_g_position", GeometryBuffer::TextureType::POSITION);
	geometry_buffer.bind_texture(&spot_light_shader, "u_g_specular", GeometryBuffer::TextureType::SPECULAR);
	geometry_buffer.bind_texture(&spot_light_shader, "u_g_diffuse", GeometryBuffer::TextureType::DIFFUSE);
	geometry_buffer.bind_texture(&spot_light_shader, "u_g_normal", GeometryBuffer::TextureType::NORMAL);

	glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	spot_light_shader.unbind();

	glCullFace(GL_BACK);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::spot_light_shadow_pass(const Scene& scene, const SpotLight& spot_light)
{
	glm::vec3 direction = spot_light.orientation * glm::vec3(0, 0, -1);
	glm::vec3 up = spot_light.orientation * glm::vec3(0, 1, 0);
	glm::mat4 light_view_mat = glm::lookAt(spot_light.position, spot_light.position + direction, up);

	glm::mat4 light_proj_view_mat = scene.camera.get_projection_matrix() * light_view_mat;	
	
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
		shadow_map.set_attribute_first_pass();		
		shadow_map.set_matrix_first_pass(light_proj_view_mat * render_data->world_mat);

		glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);

		render_data = render_data->next;
	}
	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::release()
{
}