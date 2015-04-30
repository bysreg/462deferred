#include "objmodel.hpp"
#include <SFML/System/Err.hpp>
#include <fstream>
#include <limits>
#include <iostream>
#include <map>
#include <cstdlib>

using namespace bey;

#define SKIP_THRU_CHAR( s , x ) if ( s.good() ) s.ignore( std::numeric_limits<std::streamsize>::max(), x )

// private helper function - reads a .mtl file and adds to the material table
bool ObjModel::loadMTL( std::string path, std::string filename )
{
	std::string token;
	std::string mat_name;
	std::ifstream istream( path + filename );
	if ( !istream.good( ) )
	{
		sf::err( ) << std::string( "Error opening file: " ) << path + filename << std::endl;
		return false;
	}

	// find the first material
	while ( istream.good() && token != "newmtl" ) istream >> token;
	if ( istream.eof() ) return true; // a file with no materials??

	ObjMtl material;
	istream >> mat_name;
	SKIP_THRU_CHAR( istream, '\n' );

	while ( istream.good() && (istream.peek() != EOF) )
	{
		istream >> token;

		if ( token == "newmtl" )
		{
			// push the latest material, begin a new one
			materialIDs[mat_name] = materials.size( );
			materials.push_back( material );
			material = ObjMtl();
			istream >> mat_name;		
		}
		// these are most likely the relevent materials for your renderer
		// you can do more with .obj files, but you are not expected to for p4
		else if ( token == "Ka" )
		{
			float r, g, b;
			istream >> r;
			istream >> g;
			istream >> b;
			material.Ka = glm::vec3( glm::clamp( r, 0.0f, 1.0f ),
									 glm::clamp( g, 0.0f, 1.0f ),
									 glm::clamp( b, 0.0f, 1.0f ) );
		}
		else if ( token == "Kd" )
		{
			float r, g, b;
			istream >> r;
			istream >> g;
			istream >> b;
			material.Kd = glm::vec3( glm::clamp( r, 0.0f, 1.0f ),
									 glm::clamp( g, 0.0f, 1.0f ),
									 glm::clamp( b, 0.0f, 1.0f ) );
		}
		else if ( token == "Ks" )
		{
			float r, g, b;
			istream >> r;
			istream >> g;
			istream >> b;
			material.Ks = glm::vec3( glm::clamp( r, 0.0f, 1.0f ),
									 glm::clamp( g, 0.0f, 1.0f ),
									 glm::clamp( b, 0.0f, 1.0f ) );
		}
		else if ( token == "Ns" )
		{
			istream >> material.Ns;
			material.Ns = glm::clamp( material.Ns, 0.0f, 1000.0f );
		}
		else if ( token == "map_Kd" )
		{
			istream >> token;
			// load only one copy of each texture
			if ( textureIDs.count( token ) == 0 )
			{
				textures.push_back( sf::Image() );
				if ( !textures.back().loadFromFile( path + token ) )
				{
					sf::err() << "Error loading texture: " << token << std::endl;
					return false;
				}
				textures.back().flipVertically();
				textureIDs[token] = textures.size();
			}
			material.map_Kd = textureIDs[token] - 1;
			material.map_Kd_path = token;
		}
		else if ( token == "map_Ka" )
		{
			// this is likely the same as map_Kd, but you may want to try lightmapping
			// or pre-computed radiance at some point
			istream >> token;
			if ( textureIDs.count( token ) == 0 )
			{
				textures.push_back( sf::Image( ) );
				if ( !textures.back( ).loadFromFile( path + token ) )
				{
					sf::err( ) << "Error loading texture: " << token << std::endl;
					return false;
				}
				textures.back().flipVertically();
				textureIDs[token] = textures.size( );
			}
			material.map_Ka = textureIDs[token] - 1;
		}
		// ignore all other parameters, and move to next line after each property read
		SKIP_THRU_CHAR( istream, '\n' );
	}

	// don't forget to save the last material
	materialIDs[mat_name] = materials.size( );
	materials.push_back( material );
	return true;
}

int ObjModel::get_mesh_groups_size() const
{
	return mesh_groups.size();
}

size_t ObjModel::num_vertices() const
{
	return mesh_group_vertices.size();
}

const Vertex* ObjModel::get_vertices() const
{	
	if (mesh_group_vertices.size() == 0)
		return nullptr;

	return &mesh_group_vertices[0];
}

size_t ObjModel::num_indices(int group_index) const
{
	return mesh_groups[group_index].mesh_indices.size();
}

const ObjModel::MeshGroup* ObjModel::get_mesh_group(int group_index) const
{
	return &mesh_groups[group_index];
}

const ObjModel::ObjMtl* ObjModel::get_material(int group_index) const
{
	return &materials[mesh_groups[group_index].mesh_material_id];
}

const sf::Image* ObjModel::get_texture(int texture_id) const
{
	return &textures[texture_id];
}

const unsigned int* ObjModel::get_indices(int group_index) const
{	
	return &mesh_groups[group_index].mesh_indices[0];
}

void compute_normals(Vertex* vertices, size_t num_vertices, std::vector<ObjModel::TriangleGroup>& groups)
{
	glm::vec3 zero();	
	//assume all the normals inside vertices already zero
	for (size_t j = 0; j < groups.size(); j++)
	{
		size_t num_triangles = groups[j].triangles.size();
		ObjModel::Triangle* triangles = &groups[j].triangles[0];
		
		for (size_t i = 0; i < num_triangles; i++)
		{
			unsigned int index0 = triangles[i].triangle_index[0].vertex;
			unsigned int index1 = triangles[i].triangle_index[1].vertex;
			unsigned int index2 = triangles[i].triangle_index[2].vertex;

			glm::vec3 surface_normal = glm::normalize(glm::cross(vertices[index1].position - vertices[index0].position, vertices[index2].position - vertices[index0].position));
			vertices[index0].normal += surface_normal;
			vertices[index1].normal += surface_normal;
			vertices[index2].normal += surface_normal;
		}
	}	

	//average all the surface normals
	for (size_t i = 0; i < num_vertices; i++)
	{
		vertices[i].normal = glm::normalize(vertices[i].normal);
	}
}

/*
 * Parses an input .obj file, loading data into memory.
 * This does not cover the entire .obj spec, just the most common cases, namely v/t/n triangles.
 * You will need to perform additional processing to generate meshes from the vectors of raw data.
 */
bool ObjModel::loadFromFile( std::string path, std::string filename )
{
	int total_triangles_count = 0;
	has_normal = true; // assume first that we have normal
	name = filename;

	std::string token;
	std::ifstream istream( path + filename );
	if ( !istream.good( ) )
	{
		sf::err( ) << std::string( "Error opening file: " ) << path + filename << std::endl;
		return false;
	}

	// if the .obj is in a subdirectory, .mtl files will be relative to that directory
	size_t pathlen = filename.find_last_of( "\\/", filename.npos );
	if ( pathlen < filename.npos )
		path += filename.substr( 0, pathlen + 1 );

	std::string mtl;	
	TriangleGroup group;
	Triangle triangle;
	triangle.materialID = -1;
	triangle.smoothing_group = 1;
	triangle.smooth_shading = false;

	static const char* scan_vertex = "%d";
	static const char* scan_vertex_uv = "%d/%d";
	static const char* scan_vertex_normal = "%d//%d";
	static const char* scan_vertex_uv_normal = "%d/%d/%d";

	while ( istream.good() && (istream.peek() != EOF) )
	{
		istream >> token;

		if ( token == "#" ) // comment line
		{
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "v" ) // vertex (position)
		{
			float x, y, z;
			istream >> x;
			istream >> y;
			istream >> z;
			positions.push_back( glm::vec3( x, y, z ) );
			// note: .obj supports a 'w' component, we're ignoring it here (it's very uncommon)
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "vt" ) // tex coord
		{
			float u, v;
			istream >> u;
			istream >> v;
			texcoords.push_back( glm::vec2( u, v ) );
			// similarly, .obj supports 3D textures with a 'w' component
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "vn" ) // vertex normal
		{
			float x, y, z;
			istream >> x;
			istream >> y;
			istream >> z;
			normals.push_back( glm::normalize( glm::vec3( x, y, z ) ) );
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "vp" ) // parameter space vertices, not supported
		{
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "mtllib" )
		{
			istream >> token;
			if ( !loadMTL( path, token ) )
			{
				sf::err() << "Failed to load material lib: " << token << std::endl;
				return false;
			}
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "usemtl" )
		{
			istream >> mtl;
			if ( materialIDs.count( mtl ) == 0 )
			{
				sf::err() << "Error in .obj: material \"" << mtl << "\" not found." << std::endl;
				return false;
			}
			triangle.materialID = materialIDs[mtl];
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "g" ) // starts a new group of polygons
		{
			// push the old group into the list, if it wasn't empty
			if ( group.triangles.size() > 0 )
			{
				groups.push_back( group );
				total_triangles_count += group.triangles.size();
				group.triangles.clear();
			}			
			// save the name of the group for debugging
			istream >> group.name;
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "s" ) // smoothing group index
		{
			istream >> token;
			if ( token == "off" )
				triangle.smooth_shading = false;
			else
				triangle.smoothing_group = std::stoi( token );

			// smooth shading groups is a feature of .obj used in some of the scenes
			// basically, you compute normals by averaging per-triangle normals, but only
			// for triangles in the same group. don't worry about this early on, but you may
			// need it for scenes like sponza where pre-computed normals are not provided
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "f" ) // a face, or polygon
		{
			int v, t, n;
			std::string parse;
			Triangle::VertexType format;
			std::vector<std::string> tokens;
			int temp_vertices[4];
			int temp_normals[4];
			int temp_texcoords[4];

			int peek = istream.peek();
			while (peek != '\n' && peek != EOF)
			{
				istream >> parse;
				if (istream.fail())
					break;
				tokens.push_back(parse);
				peek = istream.peek();
			}

			size_t num_vertex;
			num_vertex = tokens.size();

			if (num_vertex > 4 || num_vertex < 3) {
				std::cerr << "Syntax error, face has incorrect number of vertices" << std::endl;
				return false;
			}
						
			{
				std::string token = tokens[0];
				format = Triangle::POSITION_ONLY;

				if (token.find("//") != std::string::npos) 
				{
					format = Triangle::POSITION_NORMAL;					
				}				
				else 
				{
					size_t p1 = token.find('/');
					size_t p2 = token.rfind('/');

					if (p1 == std::string::npos) {
						format = Triangle::POSITION_ONLY;
					}else if (p1 == p2) {
						format = Triangle::POSITION_TEXCOORD;						
					}
					else {
						format = Triangle::POSITION_TEXCOORD_NORMAL;						
					}
				}

				if (format == Triangle::POSITION_ONLY || format == Triangle::POSITION_TEXCOORD)
				{
					has_normal = false;
				}
			}						

			triangle.vertexType = format;

			for (size_t i = 0; i < num_vertex; ++i) {
				switch (format)
				{
				case Triangle::POSITION_ONLY:
					sscanf(tokens[i].c_str(), scan_vertex, &temp_vertices[i]);
					temp_normals[i] = 0;
					temp_texcoords[i] = 0;								
					break;

				case Triangle::POSITION_TEXCOORD:
					sscanf(tokens[i].c_str(), scan_vertex_uv, &temp_vertices[i], &temp_texcoords[i]);					
					temp_normals[i] = 0;
					break;

				case Triangle::POSITION_NORMAL:
					sscanf(tokens[i].c_str(), scan_vertex_normal, &temp_vertices[i], &temp_normals[i]);					
					temp_texcoords[i] = 0;
					break;

				case Triangle::POSITION_TEXCOORD_NORMAL:
					sscanf(tokens[i].c_str(), scan_vertex_uv_normal, &temp_vertices[i], &temp_texcoords[i], &temp_normals[i]);
					break;

				default:
					std::cerr << "Syntax error, unrecongnized face format" << std::endl;
					break;
				}

				temp_vertices[i]--; // the obj file's index starts from 1, we adjust the index to start from zero here
				temp_normals[i]--;
				temp_texcoords[i]--;				
			}

			for (int i = 0; i < 3; i++)
			{
				triangle.triangle_index[i].vertex = temp_vertices[i];
				triangle.triangle_index[i].normal = temp_normals[i];
				triangle.triangle_index[i].texcoord = temp_texcoords[i];
			}

			group.triangles.push_back( triangle );

			if (num_vertex == 4)
			{
				triangle.triangle_index[0].vertex = temp_vertices[2];
				triangle.triangle_index[0].normal = temp_normals[2];
				triangle.triangle_index[0].texcoord = temp_texcoords[2];
				triangle.triangle_index[1].vertex = temp_vertices[3];
				triangle.triangle_index[1].normal = temp_normals[3];
				triangle.triangle_index[1].texcoord = temp_texcoords[3];
				triangle.triangle_index[2].vertex = temp_vertices[0];
				triangle.triangle_index[2].normal = temp_normals[0];
				triangle.triangle_index[2].texcoord = temp_texcoords[0];

				group.triangles.push_back(triangle);
			}

			SKIP_THRU_CHAR( istream, '\n' );
		}
		else
		{
			// ignore any other lines - invalid or unsupported obj content
			SKIP_THRU_CHAR( istream, '\n' );
		}
		token.clear();
	}

	// save the last group of polygons
	if ( group.triangles.size() > 0 )
	{
		groups.push_back(group);
		total_triangles_count += group.triangles.size();
		group.triangles.clear();
	}
	
	if ( istream.fail() && !istream.eof() )
	{
		sf::err( ) << "An error occured while reading .obj file; last token was: " << token << std::endl;
		return false;
	}

	//start turning it into a more renderer-friendly data structure
	typedef std::map< TriangleIndex, unsigned int > VertexMap;
	unsigned int vertex_last_idx = 0;
	VertexMap vertex_map;
	mesh_groups.reserve(groups.size());	
	mesh_group_vertices.reserve(total_triangles_count * 2);

	for (int i = 0; i < groups.size(); i++)
	{		
		MeshGroup mesh_group;				
		MeshIndexList& mesh_indices = mesh_group.mesh_indices;				
		mesh_indices.reserve(groups[i].triangles.size() * 3);
		mesh_group.mesh_material_id = groups[i].triangles[0].materialID; // TODO : for now, assume that all triangles inside the group all have the same material id, which actually is not always the case
		
		for (int j = 0; j < groups[i].triangles.size(); j++)
		{			
			for (int k = 0; k < 3; k++)
			{
				std::pair< VertexMap::iterator, bool > vertex = vertex_map.insert(std::make_pair(groups[i].triangles[j].triangle_index[k], vertex_last_idx));

				// if we have never seen that combination before, add it to the vertex_map, after that create new vertex based on that TriangleIndex to this group's vertices
				if (vertex.second)
				{
					Vertex mesh_vertex;
					int position_index = groups[i].triangles[j].triangle_index[k].vertex;
					int normal_index = groups[i].triangles[j].triangle_index[k].normal;
					int texcoord_index = groups[i].triangles[j].triangle_index[k].texcoord;
					mesh_vertex.position = positions[position_index];						
					mesh_vertex.normal = normal_index == -1 ? glm::vec3() : normals[normal_index];
					mesh_vertex.tex_coord = texcoord_index == -1 ? glm::vec2() : texcoords[texcoord_index];										
					mesh_group_vertices.push_back(mesh_vertex);
					++vertex_last_idx;
				}

				mesh_indices.push_back(vertex.first->second);
			}
		}
		
		mesh_groups.push_back(mesh_group);
	}

	if (!has_normal)
		compute_normals(&mesh_group_vertices[0], mesh_group_vertices.size(), groups);

	return true;
}