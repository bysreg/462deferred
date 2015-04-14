#ifndef _OBJMODEL_H_
#define _OBJMODEL_H_

#include "scene/Vertex.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <SFML/Graphics/Image.hpp>

namespace bey
{
	class ObjModel
	{
	public:

		struct ObjMtl
		{
			// the most relevant material values for basic lighting
			glm::vec3 Ka;
			glm::vec3 Kd;
			glm::vec3 Ks;
			float Ns; // specular exponent in [0,1000]

			// indexes into the textures array; -1 for no texture
			int map_Kd;
			int map_Ka;

			ObjMtl() : Ka(glm::vec3(0.0f, 0.0f, 0.0f)),
				Kd(glm::vec3(0.0f, 0.0f, 0.0f)),
				Ks(glm::vec3(0.0f, 0.0f, 0.0f)),
				Ns(0.0f),
				map_Kd(-1),
				map_Ka(-1)
			{
			}
		};

		struct TriangleIndex
		{
			int vertex; // intentionally int, to allow for negative value to indicate that we dont have that index for that variable
			int texcoord;
			int normal;

			bool operator<(const TriangleIndex& rhs) const {
				if (vertex == rhs.vertex) {
					if (normal == rhs.normal) {
						return texcoord < rhs.texcoord;
					}
					else {
						return normal < rhs.normal;
					}
				}
				else {
					return vertex < rhs.vertex;
				}
			}
		};

		// you can extend this to support quads or arbitrary polygons
		// alternatively, break quads into triangles immediately during file read
		struct Triangle
		{
			// generally, all faces in a group will use the same data, but this isn't guaranteed
			// you will want to form meshes from only faces with the same per-vertex data for building vertex buffers
			enum VertexType { POSITION_ONLY, POSITION_TEXCOORD, POSITION_NORMAL, POSITION_TEXCOORD_NORMAL } vertexType;

			int materialID; // groups can contain polygons with different materials - you may want to split these
			// into separate meshes for rendering

			bool smooth_shading;
			int smoothing_group; // smoothing group, for computing normals

			// these are indexes into the master lists of position, texcoord, and normal data
			// note 1: the starter code adjusts these to index from 0, not from 1 as in the .obj text
			// note 2: a major issue with .obj is that each vertex in a face can have unique indexes for its position,
			//         texcoord, and normal. how can you consolidate these with minimal duplication?			
			TriangleIndex triangle_index[3];
		};		

		struct TriangleGroup
		{
			std::string name;
			std::vector<Triangle> triangles;
		};

		typedef std::vector< Vertex > MeshVertexList;
		typedef std::vector< unsigned int > MeshIndexList;

		struct MeshGroup
		{
			std::string name;
			std::vector<Vertex> mesh_vertices;
			std::vector<unsigned int> mesh_indices;
		};

		int get_mesh_groups_size() const;
		size_t num_vertices(int group_index) const;
		const Vertex* get_vertices(int group_index) const;
		size_t num_indices(int group_index) const;
		const unsigned int* get_indices(int group_index) const;

		bool loadFromFile(std::string path, std::string filename);

	private:
		std::string name;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> texcoords;
		std::vector<glm::vec3> normals;		
						
		std::vector<MeshGroup> mesh_groups; // contain the compact representation of the data that the rendered needs

		// you may want to hoist materials out of models into a global scene list
		// that way multiple .obj's can inherit the same .mtl without duplication
		std::vector<ObjMtl> materials;
		std::unordered_map<std::string, int> materialIDs;
		std::vector<sf::Image> textures; // also take a look at sf::Texture - what is different about them?
		std::unordered_map<std::string, int> textureIDs;

		std::vector<TriangleGroup> groups;

		bool loadMTL(std::string path, std::string filename);
	};
}

#endif // _OBJMODEL_H_