#pragma once

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/mesh_render_info.h>

#include "Trees.h"
#include "landscape/Landscape.h"

#include "renderers/deferred_renderer.h"
#include "renderers/flat_color_renderer.h"

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

	deferred_render_style deferred_style;
	flat_color_render_style flat_color_style;

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
};
