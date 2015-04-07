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

}

void Renderer::release()
{

}