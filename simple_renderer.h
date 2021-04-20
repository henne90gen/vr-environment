#pragma once

#include <cgv_gl/surface_renderer.h>

class simple_renderer;

//! reference to a singleton spline tube renderer that is shared among drawables
/*! the second parameter is used for reference counting. Use +1 in your init method,
	-1 in your clear method and default 0 argument otherwise. If internal reference
	counter decreases to 0, singelton renderer is destructed. */
extern simple_renderer &ref_simple_renderer(cgv::render::context &ctx, int ref_count_change = 0);

struct simple_render_style : public cgv::render::surface_render_style {
	/// construct with default values
	simple_render_style() = default;
};

/// renderer that supports point splatting
class simple_renderer : public cgv::render::surface_renderer {
  protected:
	/// overload to allow instantiation of simple_renderer
	cgv::render::render_style *create_render_style() const override;

  public:
	/// initializes position_is_center to true
	simple_renderer() = default;

	/// construct shader programs and return whether this was successful, call inside of init method of drawable
	bool init(cgv::render::context &ctx) override;
	bool validate_attributes(const cgv::render::context &ctx) const override;
	bool enable(cgv::render::context &ctx) override;
	bool disable(cgv::render::context &ctx) override;
	/// convenience function to render with default settings
	void draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips = false,
			  bool use_adjacency = false, uint32_t strip_restart_index = -1) override;
};

struct simple_render_style_reflect : public simple_render_style {
	bool self_reflect(cgv::reflect::reflection_handler &rh);
};
extern cgv::reflect::extern_reflection_traits<simple_render_style, simple_render_style_reflect>
get_reflection_traits(const simple_render_style &);
