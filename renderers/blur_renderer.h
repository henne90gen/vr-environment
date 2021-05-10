#pragma once

#include <cgv/render/frame_buffer.h>
#include <cgv/render/texture.h>
#include <cgv_gl/surface_renderer.h>

struct blur_render_style : public cgv::render::surface_render_style {
	/// construct with default values
	blur_render_style() = default;
};

/// renderer that allows blurring of a texture
class blur_renderer : public cgv::render::surface_renderer {
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

	cgv::render::frame_buffer fb;

	cgv::render::texture *texture_to_be_blurred = nullptr;
	cgv::render::texture *blurred_texture = nullptr;

  protected:
	/// overload to allow instantiation of blur_renderer
	cgv::render::render_style *create_render_style() const override;

  public:
	/// initializes position_is_center to true
	blur_renderer() = default;

	/// construct shader programs and return whether this was successful, call inside of init method of drawable
	bool init(cgv::render::context &ctx) override;
	bool validate_attributes(const cgv::render::context &ctx) const override;
	bool enable(cgv::render::context &ctx) override;
	bool disable(cgv::render::context &ctx) override;
	/// convenience function to render with default settings
	void draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips = false,
			  bool use_adjacency = false, uint32_t strip_restart_index = -1) override;
	void render(cgv::render::context &ctx, cgv::render::texture &texture_to_be_blurred,
				cgv::render::texture &blurred_texture);
};

struct blur_render_style_reflect : public blur_render_style {
	bool self_reflect(cgv::reflect::reflection_handler &rh);
};
extern cgv::reflect::extern_reflection_traits<blur_render_style, blur_render_style_reflect>
get_reflection_traits(const blur_render_style &);

//! reference to a singleton spline tube renderer that is shared among drawables
/*! the second parameter is used for reference counting. Use +1 in your init method,
	-1 in your clear method and default 0 argument otherwise. If internal reference
	counter decreases to 0, singleton renderer is destructed. */
extern blur_renderer &ref_blur_renderer(cgv::render::context &ctx, int ref_count_change = 0);
