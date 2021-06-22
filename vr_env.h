#pragma once

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/gl/mesh_render_info.h>

#include "Trees.h"

#include "renderers/clouds_renderer.h"
#include "renderers/deferred_renderer.h"
#include "renderers/flat_color_renderer.h"
#include "renderers/terrain_renderer.h"

class vr_env : public cgv::base::node,
			   public cgv::render::drawable,
			   public cgv::gui::event_handler,
			   public cgv::gui::provider {
  private:
	std::vector<vec3> positions = {
		  {0.0, 0.0, 0.0},
		  {1.0, 0.0, 0.0},
		  {1.0, 1.0, 0.0},
		  {0.0, 1.0, 0.0},
	};
	std::vector<vec2> texcoords = {
		  {0.0, 1.0},
		  {1.0, 1.0},
		  {1.0, 0.0},
		  {0.0, 0.0},
	};
	std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

	cgv::render::box_render_style box_style = cgv::render::box_render_style();
	deferred_render_style deferred_style;
	clouds_render_style clouds_style;
	terrain_render_style terrain_style;

	ShaderToggles shaderToggles = {};
	TerrainParams terrainParams = {};
	bool show_example_visualization = true;

	cgv::render::texture test_texture;

	Trees trees = {};

  public:
	vr_env();

	void stream_help(std::ostream &os) override;
	bool handle(cgv::gui::event &e) override;
	bool init(cgv::render::context &ctx) override;
	void clear(cgv::render::context &ctx) override;
	void init_frame(cgv::render::context &ctx) override;
	void draw(cgv::render::context &ctx) override;
	void finish_draw(cgv::render::context &ctx) override;
	std::string get_type_name() const override;
	void create_gui() override;
	void on_set(void *member_ptr) override;
	void generate_new_seed();
	void add_new_noise_layer();

	void draw_example_visualization(cgv::render::context &ctx) const;
	void draw_scene(cgv::render::context &ctx);
};
