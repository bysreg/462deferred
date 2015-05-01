#include "scene.hpp"
#include "glm/gtc/quaternion.hpp"
#include <SFML/System/Err.hpp>
#include <fstream>
#include <limits>
#include <iostream>

using namespace bey;

/* Using a macro to avoid repeating excessively long, templated statement for a simple effect.
 * This takes a std::ifstream and a char and advances the ifstream until it passes the next
 * occurence of that character, or reaches the end of the file.
 *
 * Ignore the warnings about max() on Windows. They're lying, it compiles.
 */
#define SKIP_THRU_CHAR( s , x ) if ( s.good() ) s.ignore( std::numeric_limits<std::streamsize>::max(), x )

Scene::Scene()
{
}

bool Scene::loadFromFile( std::string filename )
{
	std::string path;
	size_t pathlen = filename.find_last_of( "\\/", filename.npos );
	if ( pathlen < filename.npos )
		path = filename.substr( 0, pathlen + 1 );
	else
		path = "./";

	std::string token;
	std::ifstream istream( filename );
	if ( !istream.good() )
	{
		sf::err( ) << std::string( "Error opening file." ) << filename << std::endl;
		return false;
	}

	while ( istream.good() && (istream.peek() != EOF) )
	{
		istream >> token;

		// skip comment lines
		if ( token == "#" )
		{
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "sunlight" )
		{
			SKIP_THRU_CHAR( istream, '{' );
			SKIP_THRU_CHAR( istream, '\n' );

			// read in the sunlight parameters - override any previously seen values
			while ( istream.good() && istream.peek() != '}' )
			{
				istream >> token;

				if ( token == "direction" )
				{
					float x, y, z;
					istream >> x;
					istream >> y;
					istream >> z;
					sunlight.direction = glm::normalize( glm::vec3( x, y, z ) );
				}
				else if ( token == "color" )
				{
					float r, g, b;
					istream >> r;
					istream >> g;
					istream >> b;
					sunlight.color = glm::vec3( glm::clamp( r, 0.0f, 1.0f ),
												glm::clamp( g, 0.0f, 1.0f ),
												glm::clamp( b, 0.0f, 1.0f ) );
				}
				else if ( token == "ambient" )
				{
					float a;
					istream >> a;
					sunlight.ambient = glm::clamp( a, 0.0f, 1.0f );
				}
				SKIP_THRU_CHAR( istream, '\n' );
			}
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "spotlight" )
		{
			SKIP_THRU_CHAR( istream, '{' );
			SKIP_THRU_CHAR( istream, '\n' );

			SpotLight spotlight;
			while ( istream.good( ) && istream.peek( ) != '}' )
			{
				istream >> token;

				if ( token == "position" )
				{
					float x, y, z;
					istream >> x;
					istream >> y;
					istream >> z;
					spotlight.position = glm::vec3( x, y, z );
				}
				else if ( token == "orientation" )
				{
					float a, x, y, z;
					istream >> a;
					istream >> x;
					istream >> y;
					istream >> z;
					spotlight.orientation = glm::normalize(glm::angleAxis(a, glm::vec3(x, y, z)));
				}
				else if ( token == "color" )
				{
					float r, g, b;
					istream >> r;
					istream >> g;
					istream >> b;
					spotlight.color = glm::vec3( glm::clamp( r, 0.0f, 1.0f ),
												 glm::clamp( g, 0.0f, 1.0f ),
												 glm::clamp( b, 0.0f, 1.0f ) );
				}
				else if ( token == "exponent" )
				{
					istream >> spotlight.exponent;				
				}
				else if ( token == "angle" )
				{
					float a;
					istream >> a;
					spotlight.angle = glm::clamp( a, 0.0f, 180.0f );
				}
				else if ( token == "attenuation" )
				{
					istream >> spotlight.Kc;
					istream >> spotlight.Kl;
					istream >> spotlight.Kq;
				}
				else if (token == "correction")
				{
					istream >> spotlight.correction;
				}
				else if (token == "slerp")
				{
					float a, x, y, z;
					istream >> a;
					istream >> x;
					istream >> y;
					istream >> z;
					spotlight.from = glm::normalize(glm::angleAxis(a, glm::vec3(x, y, z)));

					//the second quad
					istream >> a;
					istream >> x;
					istream >> y;
					istream >> z;
					spotlight.to = glm::normalize(glm::angleAxis(a, glm::vec3(x, y, z)));

					spotlight.is_slerping = true;
				}
				SKIP_THRU_CHAR( istream, '\n' );
			}

			//calculate the cutoff radius
			spotlight.cutoff = PointLight::calc_bounding_sphere_scale(spotlight.Kc, spotlight.Kl, spotlight.Kq, spotlight.color);
			spotlight.base_radius = spotlight.cutoff * glm::sin(glm::radians(spotlight.angle));
			spotlights.push_back( spotlight );
			SKIP_THRU_CHAR( istream, '\n' );			
		}
		else if ( token == "pointlight" )
		{
			SKIP_THRU_CHAR( istream, '{' );
			SKIP_THRU_CHAR( istream, '\n' );

			PointLight pointlight;
			while ( istream.good( ) && istream.peek( ) != '}' )
			{
				istream >> token;

				if ( token == "position" )
				{
					float x, y, z;
					istream >> x;
					istream >> y;
					istream >> z;
					pointlight.position = glm::vec3( x, y, z );
				}
				else if ( token == "color" )
				{
					float r, g, b;
					istream >> r;
					istream >> g;
					istream >> b;
					pointlight.color = glm::vec3( glm::clamp( r, 0.0f, 1.0f ),
												  glm::clamp( g, 0.0f, 1.0f ),
												  glm::clamp( b, 0.0f, 1.0f ) );
				}
				else if ( token == "velocity" )
				{
					float v;
					istream >> v;
					pointlight.velocity = glm::clamp( v, 0.0f, 1.0f );
				}
				else if ( token == "attenuation" )
				{
					istream >> pointlight.Kc;
					istream >> pointlight.Kl;
					istream >> pointlight.Kq;
				}
				SKIP_THRU_CHAR( istream, '\n' );
			}

			//calculate the cutoff radius
			pointlight.cutoff = PointLight::calc_bounding_sphere_scale(pointlight.Kc, pointlight.Kl, pointlight.Kq, pointlight.color);

			pointlights.push_back( pointlight );
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if ( token == "model" )
		{
			SKIP_THRU_CHAR( istream, '{' );
			SKIP_THRU_CHAR( istream, '\n' );

			StaticModel model;
			while ( istream.good( ) && istream.peek( ) != '}' )
			{
				istream >> token;

				if ( token == "position" )
				{
					float x, y, z;
					istream >> x;
					istream >> y;
					istream >> z;
					model.position = glm::vec3( x, y, z );
				}
				else if ( token == "orientation" )
				{
					float a, x, y, z;
					istream >> a;
					istream >> x;
					istream >> y;
					istream >> z;
					model.orientation = glm::normalize(glm::angleAxis(a, glm::vec3(x, y, z)));
				}
				else if ( token == "scale" )
				{
					float x, y, z;
					istream >> x;
					istream >> y;
					istream >> z;
					model.scale = glm::vec3( x, y, z );
				}
				else if ( token == "file" )
				{
					SKIP_THRU_CHAR( istream, '\"' );
					std::getline( istream, token, '\"' );

					// strip duplicate objects - only one copy of the model data in memory
					if ( objmodels.count( token ) == 0 && !objmodels[token].loadFromFile( path, token ) )
					{
						sf::err() << "Error reading .obj file: " << token << std::endl;
						return false;
					}
					model.model = &objmodels[token];
				}				

				SKIP_THRU_CHAR( istream, '\n' );
			}
			models.push_back( model );
			SKIP_THRU_CHAR( istream, '\n' );
		}
		else if (token == "camera")
		{
			SKIP_THRU_CHAR(istream, '{');
			SKIP_THRU_CHAR(istream, '\n');

			while (istream.good() && istream.peek() != '}')
			{
				istream >> token;

				if (token == "fov")
				{
					float fov;
					istream >> fov;
					camera.set_fov(fov);
				}
				else if (token == "near_clip")
				{
					float near_clip;
					istream >> near_clip;
					camera.set_near_clip(near_clip);
				}
				else if (token == "far_clip")
				{
					float far_clip;
					istream >> far_clip;
					camera.set_far_clip(far_clip);
				}
				else if (token == "position")
				{
					float x, y, z;
					istream >> x;
					istream >> y;
					istream >> z;
					camera.set_position(glm::vec3(x, y, z));
				}
				else if (token == "orientation")
				{
					float a, x, y, z;					
					istream >> a;
					istream >> x;
					istream >> y;
					istream >> z;										
					camera.set_orientation(glm::normalize(glm::angleAxis(a, glm::vec3(x, y, z))));					
				}
				SKIP_THRU_CHAR(istream, '\n');
			}
		}
		else if (token == "boundingbox")
		{
			SKIP_THRU_CHAR(istream, '{');
			SKIP_THRU_CHAR(istream, '\n');

			while (istream.good() && istream.peek() != '}')
			{
				istream >> token;

				if (token == "min")
				{
					float x, y, z;
					istream >> x;
					istream >> y;
					istream >> z;
					bounding_box.min = glm::vec3(x, y, z);
				}
				else if (token == "max")
				{
					float x, y, z;
					istream >> x;
					istream >> y;
					istream >> z;
					bounding_box.max = glm::vec3(x, y, z);					
				}				
				SKIP_THRU_CHAR(istream, '\n');
			}
		}
	}

	if ( istream.fail() )
	{
		sf::err() << "An error occured while reading scene file; last token was: " << token << std::endl;
		return false;
	}

	return true;
}

Scene::~Scene()
{
}

const StaticModel* Scene::get_static_models() const
{
	if (models.size() == 0)
		return nullptr;
	return &(models[0]);	
}

size_t Scene::num_static_models() const
{
	return models.size();
}

const DirectionalLight& Scene::get_sunlight() const
{
	return sunlight;
}

const PointLight* Scene::get_point_lights() const
{
	if (pointlights.size() == 0)
		return nullptr;
	return &pointlights[0];
}

PointLight* Scene::get_mutable_point_lights()
{
	if (pointlights.size() == 0)
		return nullptr;
	return &pointlights[0];
}

size_t Scene::num_point_lights() const
{
	return pointlights.size();
}

const SpotLight* Scene::get_spot_lights() const
{
	if (spotlights.size() == 0)
		return nullptr;
	return &spotlights[0];
}

SpotLight* Scene::get_mutable_spot_lights()
{
	if (spotlights.size() == 0)
		return nullptr;
	return &spotlights[0];
}

size_t Scene::num_spot_lights() const
{
	return spotlights.size();
}


float PointLight::calc_bounding_sphere_scale(float Kc, float Kl, float Kq, const glm::vec3& color)
{
	float max_channel = fmax(fmax(color.x, color.y), color.z);

	if (Kq <= 0)
	{
		return (256 * max_channel - Kc) / Kl;
	}

	//quadratic equation solution
	float ret = (-Kl + sqrtf((Kl * Kl) - (4 * Kq * (Kc - 256 * max_channel))))
		/
		(2 * Kq);
	return ret;
}

BoundingBox StaticModel::get_bounding_box() const
{
	BoundingBox ret;
	size_t num_vertices = model->num_vertices();
	const Vertex* vertices = model->get_vertices();

	//assume the model has at least on vertex
	ret.min = vertices[0].position;
	ret.max = vertices[0].position;

	for (int i = 1; i < num_vertices; i++)
	{
		const glm::vec3& cur_pos = vertices[i].position;
		ret.min.x = std::min(ret.min.x, cur_pos.x);
		ret.min.y = std::min(ret.min.y, cur_pos.y);
		ret.min.z = std::min(ret.min.z, cur_pos.z);

		ret.max.x = std::max(ret.max.x, cur_pos.x);
		ret.max.y = std::max(ret.max.y, cur_pos.y);
		ret.max.z = std::max(ret.max.z, cur_pos.z);
	}

	return ret;
}