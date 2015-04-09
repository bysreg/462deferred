#include "renderer.hpp"
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <iostream>

using namespace bey;

static const glm::vec3 pos = glm::vec3(0, 0, -2);

bool Renderer::initialize( const Camera& camera, const Scene& scene, const RendererInitData& data )
{
	glViewport(0, 0, data.screen_width, data.screen_height);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);	

	return true;
}

void Renderer::initialize_static_models(const StaticModel* static_models, size_t num_static_models)
{
	for (size_t i = 0; i < num_static_models; i++)
	{
		GLuint vertices_id;
		GLuint indices_id;
		GLuint normals_id;

		glGenBuffers(1, &vertices_id);
	}
}

void Renderer::render( const Camera& camera, const Scene& scene )
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec3 cameraTarget = camera.get_position() + camera.get_direction();	

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(camera.get_fov_degrees(), camera.get_aspect_ratio(), camera.get_near_clip(), camera.get_far_clip());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera.get_position().x, camera.get_position().y, camera.get_position().z, cameraTarget.x, cameraTarget.y, cameraTarget.z, camera.get_up().x, camera.get_up().y, camera.get_up().z);

	const StaticModel* staticModels = scene.get_static_models();
	for (int i = 0; i < scene.num_static_models(); i++)
	{
		glPushMatrix();

		glTranslatef(staticModels[i].position.x, staticModels[i].position.y, staticModels[i].position.y);
		//glRotatef(staticModels[i].orientation.x, staticModels[i].orientation.y, staticModels[i].position.z);
		glScalef(staticModels[i].scale.x, staticModels[i].scale.y, staticModels[i].scale.z);
		
		render_static_model(staticModels[i]);

		glPopMatrix();
	}

	{
		glPushMatrix();
		//simple triangle
		glTranslated(pos.x, pos.y, pos.z);
		glScaled(0.5f, 0.5f, 0.5f);

		glBegin(GL_TRIANGLES);
		glColor3f(0, 1, 0);
		glVertex3f(-1, -1, 0);
		glColor3f(1, 0, 0);
		glVertex3f(1, -1, 0);
		glColor3f(0, 0, 1);
		glVertex3f(0, 1, 0);
		glEnd();

		glPopMatrix();
	}
	
}

void Renderer::render_static_model(const StaticModel& static_model)
{

}

void Renderer::release()
{
	
}