#include "camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SFML/Window.hpp>
#include <iostream>

using namespace bey;

static const glm::vec3 UNIT_X = glm::vec3(1, 0, 0);
static const glm::vec3 UNIT_Y = glm::vec3(0, 1, 0);
static const glm::vec3 UNIT_Z = glm::vec3(0, 0, 1);
static const glm::vec3 ZERO = glm::vec3(0, 0, 0);
static const float translation_speed = 2;

Camera::Camera() :	proj_mat(glm::perspective(45.0f, 1.25f, 0.1f, 1000.0f)),
					position(ZERO), 
					fov(glm::pi<float>() / 4.0), 
					aspect_ratio(1), 
					near_clip(0.1f), 
					far_clip(10)
{

	view_mat = glm::lookAt(position, position + get_direction(), get_up());

}

Camera::Camera( float fovy, float aspect, float near, float far )
	:	proj_mat(glm::perspective(fovy, aspect, near, far)),
		position(ZERO)	  
{
}

Camera::~Camera()
{
}

// get a read-only handle to the projection matrix
const glm::mat4& Camera::get_projection_matrix() const
{
	return proj_mat;
}

glm::mat4 Camera::get_view_matrix() const
{
	return view_mat;
}

const glm::vec3 Camera::get_position() const
{
	return position;
}

const glm::vec3 Camera::get_direction() const
{
	return orientation * -UNIT_Z;
}

const glm::vec3 Camera::get_up() const
{
	return orientation * UNIT_Y;
}

float Camera::get_near_clip() const
{
	return near_clip;
}

float Camera::get_far_clip() const
{
	return far_clip;
}

float Camera::get_fov_radians() const
{
	return fov;
}

float Camera::get_fov_degrees() const
{
	return (180.0f * fov) / glm::pi<float>();
}

float Camera::get_aspect_ratio() const
{
	return aspect_ratio;
}

void Camera::translate(const glm::vec3& direction)
{
	position += orientation * direction;
}

void Camera::handle_input(float deltaTime)
{	
	move_direction = ZERO;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		move_direction.x = -1;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		move_direction.z = 1;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		move_direction.x = 1;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		move_direction.z = -1;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
	{
		move_direction.y = -1;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
	{
		move_direction.y = 1;
	}
}

void Camera::update(float delta)
{	
	float dist = translation_speed * delta;

	translate(move_direction * delta);
}