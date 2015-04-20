#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace bey
{
	class Camera {
	private:
		glm::mat4 proj_mat;
		glm::mat4 view_mat;
		
		glm::vec3 position;
		glm::quat orientation;		
		float fov; // in radians
		float aspect_ratio;
		float near_clip;
		float far_clip;

		//movement
		glm::vec3 move_direction;
		glm::vec3 rotate_direction;

		void update_proj();
		void update_view();
	public:

		Camera();
		Camera(float fovy, float aspect, float near, float far);
		~Camera();

		const glm::mat4& get_projection_matrix() const;
		glm::mat4 get_view_matrix() const;
		const glm::vec3 get_position() const;
		const glm::vec3 get_direction() const;
		const glm::vec3 get_up() const;
		float get_near_clip() const;
		float get_far_clip() const;
		float get_fov_radians() const;	
		float get_fov_degrees() const;
		float get_aspect_ratio() const;
		
		//setter
		void set_near_clip(float value);
		void set_far_clip(float value);
		void set_position(glm::vec3 value);
		void set_orientation(glm::quat value);
		void set_aspect_ratio(float value);
		void set_fov(float value);

		//camera control
		void translate(const glm::vec3& direction);
		void rotate(const glm::vec3& axis, float radians);

		void handle_input(float deltaTime);
		void update(float delta);
	};
}

#endif // #ifndef _CAMERA_H_