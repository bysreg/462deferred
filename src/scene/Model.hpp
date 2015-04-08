#pragma once

#include <glm/vec3.hpp>
#include <scene/Vertex.hpp>

namespace bey
{

	class Model
	{		
	private:
		size_t vertices_count;
		size_t indices_count;

	public:
		Model();
		~Model();

		Vertex* vertices;
		unsigned int* indices;

		const Vertex* get_vertices() const;
		const unsigned int* get_indices() const;
		size_t num_vertices() const;
		size_t num_indices() const;

		void initialize();
	};

}