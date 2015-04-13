#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <renderer/camera.hpp>
#include <renderer/Shader.hpp>
#include <renderer/RendererInitData.hpp>
#include <scene/scene.hpp>
#include <vector>
#include <GL/glew.h>

namespace bey
{
	struct RenderData
	{
		GLuint vertices_id;
		GLuint indices_id;
		const StaticModel* model;
		glm::mat4x4 world_mat;
		int group_id;
		RenderData* next;
		bool is_dirty;

		RenderData() : vertices_id(0), indices_id(0), model(nullptr), group_id(-1), next(nullptr), is_dirty(true) {}
	};

	class Renderer {
	private:

		std::vector< std::vector< RenderData> > render_datas; // each model and each group has its own render_data
		std::vector< Shader > shaders;

	public:

		// You may want to build some scene-specific OpenGL data before the first frame
		bool initialize(const Camera& camera, const Scene& scene, const RendererInitData& data);

		void initialize_shaders();
		void initialize_static_models(const StaticModel* static_models, size_t num_static_models);

		/*
		 * Render a frame to the currently active OpenGL context.
		 * It's best to keep all your OpenGL-specific data in the renderer; keep the Scene class clean.
		 * This function should not modify the scene or camera.
		 */
		void render(const Camera& camera, const Scene& scene);		

		// release all OpenGL data and allocated memory
		// you can do this in the destructor instead, but a callable function lets you swap scenes at runtime
		void release();

	};
}

#endif // #ifndef _RENDERER_H_