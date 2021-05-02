#pragma once

#include <cgv_gl/surface_renderer.h>

class flat_color_renderer;

//! reference to a singleton spline tube renderer that is shared among drawables
/*! the second parameter is used for reference counting. Use +1 in your init method,
	-1 in your clear method and default 0 argument otherwise. If internal reference
	counter decreases to 0, singelton renderer is destructed. */
extern flat_color_renderer &ref_flat_color_renderer(cgv::render::context &ctx, int ref_count_change = 0);

struct flat_color_render_style : public cgv::render::surface_render_style {
	/// construct with default values
	flat_color_render_style() = default;
};

/// renderer that supports point splatting
class flat_color_renderer : public cgv::render::surface_renderer {
  private:
	bool has_texture = false;
	cgv::render::texture texture;
  protected:
	/// overload to allow instantiation of flat_color_renderer
	cgv::render::render_style *create_render_style() const override;

  public:
	/// initializes position_is_center to true
	flat_color_renderer() = default;

	/// construct shader programs and return whether this was successful, call inside of init method of drawable
	bool init(cgv::render::context &ctx) override;
	bool validate_attributes(const cgv::render::context &ctx) const override;
	bool enable(cgv::render::context &ctx) override;
	bool disable(cgv::render::context &ctx) override;
	/// convenience function to render with default settings
	void draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips = false,
			  bool use_adjacency = false, uint32_t strip_restart_index = -1) override;
	void set_texture(cgv::render::context &ctx, const cgv::render::texture &t);
};

struct flat_color_render_style_reflect : public flat_color_render_style {
	bool self_reflect(cgv::reflect::reflection_handler &rh);
};
extern cgv::reflect::extern_reflection_traits<flat_color_render_style, flat_color_render_style_reflect>
get_reflection_traits(const flat_color_render_style &);
