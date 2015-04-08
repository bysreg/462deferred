#include <scene/Model.hpp>

using namespace bey;

Model::Model()
{

}

Model::~Model()
{

}

const Vertex* Model::get_vertices() const
{
	return vertices;
}

const unsigned int* Model::get_indices() const
{
	return indices;
}

size_t Model::num_vertices() const
{
	return vertices_count;
}

size_t Model::num_indices() const
{
	return indices_count;
}