#pragma once

#include <cgv_gl/surface_renderer.h>
#include <util/OpenGLUtils.h>

class deferred_renderer;

//! reference to a singleton spline tube renderer that is shared among drawables
/*! the second parameter is used for reference counting. Use +1 in your init method,
	-1 in your clear method and default 0 argument otherwise. If internal reference
	counter decreases to 0, singelton renderer is destructed. */
extern deferred_renderer &ref_deferred_renderer(cgv::render::context &ctx, int ref_count_change = 0);

struct deferred_render_style : public cgv::render::surface_render_style {
	/// construct with default values
	deferred_render_style() = default;
};

/// renderer that supports point splatting
class deferred_renderer : public cgv::render::surface_renderer {
  private:
    GLuint gBuffer = 0;
    GLuint gPosition = 0;
    GLuint gNormal = 0;
    GLuint gAlbedo = 0;
    GLuint gDoLighting = 0;
    GLuint depthBuffer = 0;

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
	/// convenience function to render with default settings
	void draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips = false,
			  bool use_adjacency = false, uint32_t strip_restart_index = -1) override;
};

struct deferred_render_style_reflect : public deferred_render_style {
	bool self_reflect(cgv::reflect::reflection_handler &rh);
};
extern cgv::reflect::extern_reflection_traits<deferred_render_style, deferred_render_style_reflect>
get_reflection_traits(const deferred_render_style &);
