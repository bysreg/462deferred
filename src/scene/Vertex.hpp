#pragma once

#include <glm/glm.hpp>

namespace bey
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec4 color;
		glm::vec3 normal;
		glm::vec2 tex_coord;		
	};
}
