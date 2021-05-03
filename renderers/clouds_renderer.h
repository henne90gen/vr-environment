#pragma once

#include <cgv_gl/surface_renderer.h>

class clouds_renderer;

//! reference to a singleton spline tube renderer that is shared among drawables
/*! the second parameter is used for reference counting. Use +1 in your init method,
	-1 in your clear method and default 0 argument otherwise. If internal reference
	counter decreases to 0, singleton renderer is destructed. */
extern clouds_renderer &ref_clouds_renderer(cgv::render::context &ctx, int ref_count_change = 0);

struct clouds_render_style : public cgv::render::surface_render_style {
	/// construct with default values
	clouds_render_style() = default;

	bool enabled = true;
	float cloud_blend = 0.1F;
	float animation_speed = 1.0F;
	float animation_time = 0.0F;

	const float HALF_PI = 1.570796327;
	vec3 clouds_position = vec3(0.0F, 200.0F, 0.0F);
	vec3 clouds_rotation = vec3(HALF_PI, 0.0F, 0.0F);
	vec3 clouds_scale = vec3(2000.0F, 2000.0F, 1.0F);
};

/// renderer that supports point splatting
class clouds_renderer : public cgv::render::surface_renderer {
  private:
	std::vector<vec3> positions = {
		  {-1.0, -1.0, 0.0},
		  {1.0, -1.0, 0.0},
		  {1.0, 1.0, 0.0},
		  {-1.0, 1.0, 0.0},
	};
	std::vector<vec2> texcoords = {
		  {0.0, 1.0},
		  {1.0, 1.0},
		  {1.0, 0.0},
		  {0.0, 0.0},
	};
	std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

  protected:
	/// overload to allow instantiation of clouds_renderer
	cgv::render::render_style *create_render_style() const override;

  public:
	/// initializes position_is_center to true
	clouds_renderer() = default;

	/// construct shader programs and return whether this was successful, call inside of init method of drawable
	bool init(cgv::render::context &ctx) override;
	bool validate_attributes(const cgv::render::context &ctx) const override;
	bool enable(cgv::render::context &ctx) override;
	bool disable(cgv::render::context &ctx) override;
	/// convenience function to render with default settings
	void draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips = false,
			  bool use_adjacency = false, uint32_t strip_restart_index = -1) override;
	void render(cgv::render::context &ctx);
};

struct clouds_render_style_reflect : public clouds_render_style {
	bool self_reflect(cgv::reflect::reflection_handler &rh);
};
extern cgv::reflect::extern_reflection_traits<clouds_render_style, clouds_render_style_reflect>
get_reflection_traits(const clouds_render_style &);
