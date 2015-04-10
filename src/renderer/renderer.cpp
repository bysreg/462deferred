#include "renderer.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <iostream>

using namespace bey;

static const glm::vec3 pos = glm::vec3(0, 0, -2);
static const glm::mat4x4 trans_mat = glm::mat4x4();

GLuint vertices_id;
GLuint indices_id;
Vertex test_vertices[3];
unsigned int test_indices[3];

bool Renderer::initialize( const Camera& camera, const Scene& scene, const RendererInitData& data )
{
	glViewport(0, 0, data.screen_width, data.screen_height);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);	

	initialize_static_models(scene.get_static_models(), scene.num_static_models());
	initialize_shaders();

	return true;
}

void Renderer::initialize_shaders()
{
	Shader shader;

	shader.load_shader_program("../../shaders/simple_triangle.vs", "../../shaders/simple_triangle.fs");
	shaders.push_back(shader);
}

void Renderer::initialize_static_models(const StaticModel* static_models, size_t num_static_models)
{	
	test_vertices[0].position = glm::vec3(-1, -1, 0);
	test_vertices[0].color = glm::vec4(0, 1, 0, 1);
	test_vertices[1].position = glm::vec3(1, -1, 0);
	test_vertices[1].color = glm::vec4(1, 0, 0, 1);
	test_vertices[2].position = glm::vec3(0, 1, 0);
	test_vertices[2].color = glm::vec4(0, 0, 1, 1);

	test_indices[0] = 0;
	test_indices[1] = 1;
	test_indices[2] = 2;

	/*for (size_t i = 0; i < num_static_models; i++)
	{
		const StaticModel& static_model = static_models[i];
		for (int j = 0; j < static_model.model->get_mesh_groups_size(); j++)
		{*/
			const Vertex* vertices = /*static_model.model->get_vertices(j);*/ test_vertices;
			const unsigned int* indices = /*static_model.model->get_indices(j);*/ test_indices;
			size_t vertices_size =/* static_model.model->get_vertices_size(j);*/ 3;
			size_t indices_size = /*static_model.model->get_indices_size(j);*/ 3;
			//GLuint vertices_id; // use test global for now. will fix later
			//GLuint indices_id;

			glGenBuffers(1, &vertices_id);
			glGenBuffers(1, &indices_id);

			glBindBuffer(GL_ARRAY_BUFFER, vertices_id);
			glBufferData(GL_ARRAY_BUFFER, vertices_size * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		/*}		
	}*/
}

void Renderer::render( const Camera& camera, const Scene& scene )
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec3 cameraTarget = camera.get_position() + camera.get_direction();	

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);	

	//------------------

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(camera.get_fov_degrees(), camera.get_aspect_ratio(), camera.get_near_clip(), camera.get_far_clip());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera.get_position().x, camera.get_position().y, camera.get_position().z, cameraTarget.x, cameraTarget.y, cameraTarget.z, camera.get_up().x, camera.get_up().y, camera.get_up().z);

	//----------------

	//shaders[0].bind(); // fixme : will fix it with the appropriate shader

	//const StaticModel* static_models = scene.get_static_models();
	////for (int i = 0; i < scene.num_static_models(); i++)
	////{
	////	const StaticModel& static_model = static_models[i];
	////	for (int j = 0; j < static_model.model->get_mesh_groups_size(); j++)
	////	{			
	////		render_static_model(static_model, j, shaders[0], camera);			
	////	}		
	////}	

	//render_static_model(static_models[0], 0, shaders[0], camera);

	//shaders[0].unbind();

	//------

	shaders[0].bind();

	for (int model_index = 0; model_index < scene.num_static_models(); model_index++)
	{			
		const StaticModel& static_model = scene.get_static_models()[model_index];

		glPushMatrix();
		glTranslated(static_model.position.x, static_model.position.y, static_model.position.z);
		glScaled(static_model.scale.x, static_model.scale.y, static_model.scale.z);

		int model_group = 0;
				
		const Vertex* vertices = static_model.model->get_vertices(0);
		const unsigned int* indices = static_model.model->get_indices(0);
		size_t vertices_size = static_model.model->get_vertices_size(0);
		size_t indices_size = static_model.model->get_indices_size(0);

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < indices_size; i++)
		{
			glColor3f(vertices[indices[i]].color.x, vertices[indices[i]].color.y, vertices[indices[i]].color.z);
			glVertex3f(vertices[indices[i]].position.x, vertices[indices[i]].position.y, vertices[indices[i]].position.z);
		}
		glEnd();

		glPopMatrix();
	}		
	
	shaders[0].unbind();
}

void Renderer::render_static_model(const StaticModel& static_model, int group_index,const Shader& shader, const Camera& camera)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertices_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

	if (shader.posL_attribute != -1)
	{
		glEnableVertexAttribArray(shader.posL_attribute);
		glVertexAttribPointer(shader.posL_attribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	}

	if (shader.color_attribute != -1)
	{
		glDisableVertexAttribArray(shader.color_attribute);
		glVertexAttribPointer(shader.color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) sizeof(glm::vec3));
	}

	//mandatory shader variables
	GLint uni_world = glGetUniformLocation(shader.program, "world");
	if (uni_world != -1) {
		glUniformMatrix4fv(uni_world, 1, GL_FALSE, glm::value_ptr(trans_mat));
	}

	GLint uni_view = glGetUniformLocation(shader.program, "view");
	if (uni_view != -1) {
		glUniformMatrix4fv(uni_view, 1, GL_FALSE, glm::value_ptr(camera.get_view_matrix()));
	}

	GLint uni_proj = glGetUniformLocation(shader.program, "proj");
	if (uni_proj != -1) {
		glUniformMatrix4fv(uni_proj, 1, GL_FALSE, glm::value_ptr(camera.get_projection_matrix()));
	}

	glDrawElements(GL_TRIANGLES, /*static_model.model->get_indices_size(group_index)*/ 3, GL_UNSIGNED_INT, 0);

	//unbind all previous binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::release()
{
	
}