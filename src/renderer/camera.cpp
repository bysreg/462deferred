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
static const float translation_speed = 4;
static const float rotation_speed = 1;

Camera::Camera() :	position(glm::vec3(0, 2, 5)), 
					fov(glm::pi<float>() / 4.0), 
					aspect_ratio(1), 
					near_clip(0.1f), 
					far_clip(100)
{

	view_mat = glm::lookAt(position, position + get_direction(), get_up());
	proj_mat = glm::perspective(get_fov_degrees(), get_aspect_ratio(), get_near_clip(), get_far_clip());
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

void Camera::set_near_clip(float value)
{
	near_clip = value;
	update_proj();
}

void Camera::set_far_clip(float value)
{
	far_clip = value;
	update_proj();
}

void Camera::set_aspect_ratio(float value)
{
	aspect_ratio = value;
	update_proj();
}

void Camera::set_fov(float value)
{
	fov = value;
	update_proj();
}

void Camera::set_position(glm::vec3 value)
{
	position = value;
	update_view();
}

void Camera::set_orientation(glm::quat value)
{
	orientation = value;
	update_view();
}

void Camera::translate(const glm::vec3& direction)
{	
	set_position(position + orientation * direction);
}

void Camera::rotate(const glm::vec3& axis, float radians)
{
	set_orientation(glm::normalize(glm::angleAxis(radians, axis) * orientation));
}

void Camera::handle_input(float deltaTime)
{	
	move_direction = ZERO;
	rotate_direction = ZERO;
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

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::J))
	{
		rotate_direction.y = 1;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::L))
	{
		rotate_direction.y = -1;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::I))
	{
		rotate_direction.x = 1;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::K))
	{
		rotate_direction.x = -1;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
	{
		std::cout << position.x << " " << position.y << " " << position.z << std::endl;
		glm::vec3 axis = glm::axis(orientation);
		std::cout << glm::angle(orientation) << " " << axis.x << " " << axis.y << " " << axis.z << std::endl;
	}
}

void Camera::update(float delta)
{		
	translate(move_direction * delta * translation_speed);

	if (rotate_direction.y != 0)
	{
		rotate(UNIT_Y * rotate_direction.y, delta * rotation_speed); // rotate around the global Y axis
	}	

	if (rotate_direction.x != 0)
	{
		rotate(orientation * UNIT_X * rotate_direction.x, delta * rotation_speed); // rotate around the local X axis
	}
}

void Camera::update_proj()
{
	proj_mat = glm::perspective(get_fov_degrees(), get_aspect_ratio(), get_near_clip(), get_far_clip());
}

void Camera::update_view()
{
	view_mat = glm::lookAt(position, position + get_direction(), get_up());
}