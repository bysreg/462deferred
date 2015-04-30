#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "renderer/camera.hpp"
#include "renderer/Shader.hpp"
#include "renderer/RendererInitData.hpp"
#include "renderer/GeometryBuffer.hpp"
#include "renderer/ShadowMap.hpp"
#include "scene/scene.hpp"
#include <vector>
#include <GL/glew.h>

namespace bey
{
	struct RenderData
	{
		GLuint vertices_id;
		GLuint indices_id;
		const StaticModel* model;
		GLuint diffuse_texture_id;
		const ObjModel::ObjMtl* material;
		glm::mat4x4 world_mat;
		int group_id; // every vertices in a group is guaranteed to have the same material id
		BoundingBox bounding_box; // in world position
		RenderData* next;		

		RenderData() : vertices_id(0), indices_id(0), model(nullptr), group_id(-1), next(nullptr) {}
	};

	class Renderer {
	private:

		std::vector< std::vector< RenderData> > render_datas; // each model and each group has its own render_data
		std::vector< Shader > shaders;		
		std::unordered_map<std::string, GLuint> texture_ids;
		RenderData* head;
		GeometryBuffer geometry_buffer;
		ShadowMap shadow_map;
		Shader directional_light_shader;
		Shader point_light_shader;
		Shader spot_light_shader;
		Shader stencil_shader;		

		int screen_width;
		int screen_height;

		RenderData* quad;
		RenderData* sphere;
		RenderData* cone;

	public:

		// You may want to build some scene-specific OpenGL data before the first frame
		bool initialize(const Scene& scene, const RendererInitData& data);
		void initialize_primitives();
		void initialize_shaders();
		void initialize_static_models(const StaticModel* static_models, size_t num_static_models);
		void initialize_material(const StaticModel& static_model, int group_index, RenderData& render_data);

		//general shader
		void set_attributes(const Shader& shader);
		void set_uniforms(GLuint shader_program, const RenderData& render_data, const Camera& camera);

		/*
		 * Render a frame to the currently active OpenGL context.
		 * It's best to keep all your OpenGL-specific data in the renderer; keep the Scene class clean.
		 * This function should not modify the scene or camera.
		 */
		void render(const Camera& camera, const Scene& scene);		
		void geometry_pass(const Scene& scene);
		void begin_light_pass(const Scene& scene);
		void end_light_pass(const Scene& scene);
		void stencil_pass(const Scene& scene, const RenderData& render_data);
		void directional_light_pass(const Scene& scene);		
		void point_light_pass(const Scene& scene, const PointLight& point_light);
		void spot_light_pass(const Scene& scene, const SpotLight& spot_light);
		void render_model(const Camera& camera, const Scene& scene, const RenderData& render_data, const Shader& shader);				

		void render_all_models(const Camera& camera, const Scene& scene);

		//shadow passes
		void directional_light_shadow_pass(const Scene& scene);
		void spot_light_shadow_pass(const Scene& scene, const SpotLight& spot_light);

		RenderData* create_quad();
		RenderData* create_sphere();
		RenderData* create_cone();

		// release all OpenGL data and allocated memory
		// you can do this in the destructor instead, but a callable function lets you swap scenes at runtime
		void release();
	};
}

#endif // #ifndef _RENDERER_H_