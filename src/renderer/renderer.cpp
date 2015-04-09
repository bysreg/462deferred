#include "renderer.hpp"
#include <glm/glm.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>

using namespace bey;

bool Renderer::initialize( const Camera& camera, const Scene& scene, const RendererInitData& data )
{
	glViewport(0, 0, data.screen_width, data.screen_height);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	return true;
}

static const glm::vec3 pos = glm::vec3(0, 0, -2);

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

	glPushMatrix();

	glTranslated(pos.x, pos.y, pos.z);	
	glScaled(0.5f, 0.5f, 0.5f);

	const StaticModel* staticModels = scene.get_static_models();
	for (int i = 0; i < scene.num_static_models(); i++)
	{
		//glTranslatef(staticModels.)
		//staticModels[i].
	}

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

void Renderer::release()
{
	
}