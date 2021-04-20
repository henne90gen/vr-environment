#include "simple_renderer.h"

#include <cgv_gl/gl/gl_tools.h>

simple_renderer &ref_simple_renderer(cgv::render::context &ctx, int ref_count_change) {
	static int ref_count = 0;
	static simple_renderer r;
	r.manage_singelton(ctx, "simple_renderer", ref_count, ref_count_change);
	return r;
}

cgv::render::render_style *simple_renderer::create_render_style() const { return new simple_render_style(); }

simple_render_style::simple_render_style() {
	radius_scale = 1.0f;
	radius = 1.0f;
}

bool simple_renderer::validate_attributes(const cgv::render::context &ctx) const {
	const auto &strs = get_style<simple_render_style>();
	return surface_renderer::validate_attributes(ctx);
}

bool simple_renderer::init(cgv::render::context &ctx) {
	bool res = surface_renderer::init(ctx);
	if (!ref_prog().is_created()) {
		if (!ref_prog().build_program(ctx, "simple.glpr", true)) {
			std::cerr << "ERROR in simple_renderer::init() ... could not build program simple.glpr" << std::endl;
			return false;
		}
	}
	return res;
}

bool simple_renderer::enable(cgv::render::context &ctx) {
	const auto &strs = get_style<simple_render_style>();

	if (!surface_renderer::enable(ctx))
		return false;

	if (!ref_prog().is_linked())
		return false;

	ref_prog().set_uniform(ctx, "radius_scale", strs.radius_scale);
	ref_prog().set_uniform(ctx, "eye_pos", eye_pos);

	return true;
}

bool simple_renderer::disable(cgv::render::context &ctx) { return surface_renderer::disable(ctx); }

bool simple_render_style_reflect::self_reflect(cgv::reflect::reflection_handler &rh) {
	return rh.reflect_base(*static_cast<simple_render_style *>(this)) && //
		   rh.reflect_member("radius", radius) &&                        //
		   rh.reflect_member("radius_scale", radius_scale);
}

void simple_renderer::draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
						   uint32_t strip_restart_index) {
	draw_impl(ctx, cgv::render::PT_TRIANGLES, start, count, use_strips, use_adjacency, strip_restart_index);
}

cgv::reflect::extern_reflection_traits<simple_render_style, simple_render_style_reflect>
get_reflection_traits(const simple_render_style &) {
	return cgv::reflect::extern_reflection_traits<simple_render_style, simple_render_style_reflect>();
}

#include <cgv/gui/provider.h>

struct simple_render_style_gui_creator : public cgv::gui::gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider *p, const std::string &label, void *value_ptr, const std::string &value_type,
				const std::string &gui_type, const std::string &options, bool *) override {
		if (value_type != cgv::type::info::type_name<simple_render_style>::get_name())
			return false;
		auto *strs_ptr = reinterpret_cast<simple_render_style *>(value_ptr);
		auto *b = dynamic_cast<cgv::base::base *>(p);

		p->add_member_control(b, "default radius", strs_ptr->radius, "value_slider",
							  "min=0.001;step=0.0001;max=10.0;log=true;ticks=true");
		p->add_member_control(b, "radius scale", strs_ptr->radius_scale, "value_slider",
							  "min=0.01;step=0.0001;max=100.0;log=true;ticks=true");

		p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style *>(strs_ptr));
		return true;
	}
};

cgv::gui::gui_creator_registration<simple_render_style_gui_creator> simple_rs_gc_reg("simple_render_style_gui_creator");
