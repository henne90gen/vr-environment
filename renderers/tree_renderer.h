#pragma once

#include <cgv_gl/surface_renderer.h>

class tree_renderer;

//! reference to a singleton spline tube renderer that is shared among drawables
/*! the second parameter is used for reference counting. Use +1 in your init method,
	-1 in your clear method and default 0 argument otherwise. If internal reference
	counter decreases to 0, singleton renderer is destructed. */
extern tree_renderer &ref_tree_renderer(cgv::render::context &ctx, int ref_count_change = 0);

struct tree_render_style : public cgv::render::surface_render_style {
	int tree_count = 10;
	float rotation = 0.0F;

	/// construct with default values
	tree_render_style() = default;
};

/// renderer that supports point splatting
class tree_renderer : public cgv::render::surface_renderer {
  private:
	bool has_position_texture = false;
	cgv::render::texture position_texture;

	bool has_surface_texture = false;
	cgv::render::texture surface_texture = {};

  protected:
	/// overload to allow instantiation of tree_renderer
	cgv::render::render_style *create_render_style() const override;

  public:
	/// initializes position_is_center to true
	tree_renderer() = default;

	/// construct shader programs and return whether this was successful, call inside of init method of drawable
	bool init(cgv::render::context &ctx) override;
	bool validate_attributes(const cgv::render::context &ctx) const override;
	bool enable(cgv::render::context &ctx) override;
	bool disable(cgv::render::context &ctx) override;
	/// convenience function to render with default settings
	void draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips = false,
			  bool use_adjacency = false, uint32_t strip_restart_index = -1) override;
	void set_position_texture(cgv::render::context &ctx, const cgv::render::texture &t);
	void set_surface_texture(cgv::render::context &ctx, const cgv::render::texture &t);
};

struct tree_render_style_reflect : public tree_render_style {
	bool self_reflect(cgv::reflect::reflection_handler &rh);
};
extern cgv::reflect::extern_reflection_traits<tree_render_style, tree_render_style_reflect>
get_reflection_traits(const tree_render_style &);
