#pragma once

#include <cgv/render/frame_buffer.h>
#include <cgv_gl/surface_renderer.h>

#define DEPTH_TO_TEXTURE 1

class deferred_renderer;

//! reference to a singleton spline tube renderer that is shared among drawables
/*! the second parameter is used for reference counting. Use +1 in your init method,
	-1 in your clear method and default 0 argument otherwise. If internal reference
	counter decreases to 0, singleton renderer is destructed. */
extern deferred_renderer &ref_deferred_renderer(cgv::render::context &ctx, int ref_count_change = 0);

enum class DeferredRenderTarget {
	DEFAULT = 0,
	POSITION = 1,
	NORMAL = 2,
	ALBEDO = 3,
	IS_CLOUD = 4,
	SSAO = 5,
	SSAO_BLUR = 6,
	DEPTH = 7,
	TEST = 8,
};

struct deferred_render_style : public cgv::render::surface_render_style {
	/// construct with default values
	deferred_render_style() = default;

	DeferredRenderTarget render_target = DeferredRenderTarget::DEFAULT;
	bool use_atmospheric_scattering = true;
	bool use_ambient_occlusion = true;
};

/// renderer that supports point splatting
class deferred_renderer : public cgv::render::surface_renderer {
  private:
	std::vector<vec3> positions = {
		  {-1.0, -1.0, 0.0},
		  {1.0, -1.0, 0.0},
		  {1.0, 1.0, 0.0},
		  {-1.0, 1.0, 0.0},
	};
	std::vector<vec2> texcoords = {
		  {0.0, 0.0},
		  {1.0, 0.0},
		  {1.0, 1.0},
		  {0.0, 1.0},
	};
	std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

  public:
	cgv::render::frame_buffer gBuffer;
	cgv::render::texture gPosition;
	cgv::render::texture gNormal;
	cgv::render::texture gAlbedo;
	cgv::render::texture gIsCloud;
	cgv::render::texture ssao_texture;
	cgv::render::texture blurred_ssao_texture;
#if DEPTH_TO_TEXTURE
	cgv::render::texture gDepth;
#else
	cgv::render::render_buffer gDepth;
#endif

  protected:
	/// overload to allow instantiation of deferred_renderer
	cgv::render::render_style *create_render_style() const override;

  public:
	/// initializes position_is_center to true
	deferred_renderer() = default;

	/// construct shader programs and return whether this was successful, call inside of init method of drawable
	bool init(cgv::render::context &ctx) override;
	bool validate_attributes(const cgv::render::context &ctx) const override;
	bool enable(cgv::render::context &ctx) override;
	bool disable(cgv::render::context &ctx) override;
	void render(cgv::render::context &ctx, const int viewport[4], const std::function<void()> &func);
	/// convenience function to render with default settings
	void draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips = false,
			  bool use_adjacency = false, uint32_t strip_restart_index = -1) override;

	bool init_g_buffer(cgv::render::context &ctx, int width, int height);
};

struct deferred_render_style_reflect : public deferred_render_style {
	bool self_reflect(cgv::reflect::reflection_handler &rh);
};
extern cgv::reflect::extern_reflection_traits<deferred_render_style, deferred_render_style_reflect>
get_reflection_traits(const deferred_render_style &);
