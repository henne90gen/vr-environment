#pragma once

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/mesh_render_info.h>

#include <cgv_gl/arrow_renderer.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/rounded_cone_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/spline_tube_renderer.h>

#include "landscape/Landscape.h"

class vr_env : public cgv::base::node,
			   public cgv::render::drawable,
			   public cgv::gui::event_handler,
			   public cgv::gui::provider {
  private:
	Landscape landscape = {};

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
};
