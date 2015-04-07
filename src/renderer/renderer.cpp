#include "renderer.hpp"
#include <glm/glm.hpp>
#include <SFML/OpenGL.hpp>

using namespace bey;

bool Renderer::initialize( const Camera& camera, const Scene& scene, const RendererInitData& data )
{
	glViewport(0, 0, data.screen_width, data.screen_height);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	return true;
}

void Renderer::render( const Camera& camera, const Scene& scene )
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	glColor3f(0, 1, 0);
	glVertex3f(-1, -1, 0);
	glColor3f(1, 0, 0);
	glVertex3f(1, -1, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 1, 0);
	glEnd();
}

void Renderer::release()
{

}