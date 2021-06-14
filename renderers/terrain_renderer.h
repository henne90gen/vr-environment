#pragma once

#include <cgv_gl/surface_renderer.h>

#include "../TerrainParams.h"

struct TerrainLevels {
	float grassLevel = 0.4F;
	float rockLevel = 1.0F;
	float blur = 0.05F;
};

struct terrain_render_style : public cgv::render::surface_render_style {
	/// construct with default values
	terrain_render_style() = default;

	bool enabled = true;
	bool wireframe = false;
	bool noiseLayersEnabled = true;
	bool powerEnabled = true;
	bool bowlEnabled = true;
	bool platformEnabled = true;

	TerrainLevels levels = {};
	float tessellation = 60.0F;
	float uv_scale_factor = 20.0F;
};

/// renderer that supports point splatting
class terrain_renderer : public cgv::render::surface_renderer {
  private:
	// TODO find a better way to pass TerrainParams down to enable()
	const TerrainParams *params = nullptr;

	cgv::render::texture grass_texture;
	cgv::render::texture dirt_texture;
	cgv::render::texture rock_texture;

	std::vector<vec2> custom_positions;
	std::vector<unsigned int> custom_indices;

  protected:
	/// overload to allow instantiation of terrain_renderer
	cgv::render::render_style *create_render_style() const override;

  public:
	/// initializes position_is_center to true
	terrain_renderer() = default;

	/// construct shader programs and return whether this was successful, call inside of init method of drawable
	bool init(cgv::render::context &ctx) override;
	bool validate_attributes(const cgv::render::context &ctx) const override;
	bool enable(cgv::render::context &ctx) override;
	bool disable(cgv::render::context &ctx) override;
	void draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
			  uint32_t strip_restart_index) override;
	void render(cgv::render::context &ctx, const TerrainParams &terrainParams);

  private:
	void init_positions();
};

struct terrain_render_style_reflect : public terrain_render_style {
	bool self_reflect(cgv::reflect::reflection_handler &rh);
};
extern cgv::reflect::extern_reflection_traits<terrain_render_style, terrain_render_style_reflect>
get_reflection_traits(const terrain_render_style &);

//! reference to a singleton terrain renderer that is shared among drawables
/*! the second parameter is used for reference counting. Use +1 in your init method,
	-1 in your clear method and default 0 argument otherwise. If internal reference
	counter decreases to 0, singleton renderer is destructed. */
extern terrain_renderer &ref_terrain_renderer(cgv::render::context &ctx, int ref_count_change = 0);
