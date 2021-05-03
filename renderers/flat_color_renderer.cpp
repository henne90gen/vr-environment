#include "flat_color_renderer.h"

#include <cgv_gl/gl/gl_tools.h>

flat_color_renderer &ref_flat_color_renderer(cgv::render::context &ctx, int ref_count_change) {
	static int ref_count = 0;
	static flat_color_renderer r;
	r.manage_singelton(ctx, "flat_color_renderer", ref_count, ref_count_change);
	return r;
}

cgv::render::render_style *flat_color_renderer::create_render_style() const { return new flat_color_render_style(); }

bool flat_color_renderer::validate_attributes(const cgv::render::context &ctx) const {
	const auto &strs = get_style<flat_color_render_style>();
	return surface_renderer::validate_attributes(ctx);
}

bool flat_color_renderer::init(cgv::render::context &ctx) {
	bool res = surface_renderer::init(ctx);
	if (!ref_prog().is_created()) {
		if (!ref_prog().build_program(ctx, "flat_color.glpr", true)) {
			std::cerr << "ERROR in flat_color_renderer::init() ... could not build program flat_color.glpr"
					  << std::endl;
			return false;
		}
	}
	return res;
}

bool flat_color_renderer::enable(cgv::render::context &ctx) {
	const auto &strs = get_style<flat_color_render_style>();

	if (!surface_renderer::enable(ctx)) {
		return false;
	}

	if (!ref_prog().is_linked()) {
		return false;
	}

	const auto &style = get_style<flat_color_render_style>();
	if (!ref_prog().set_uniform(ctx, "flat_color", style.surface_color)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "has_texture", has_texture)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "texture_sampler", 0)) {
		return false;
	}
	return texture.enable(ctx, 0);
}

bool flat_color_renderer::disable(cgv::render::context &ctx) { return surface_renderer::disable(ctx); }

bool flat_color_render_style_reflect::self_reflect(cgv::reflect::reflection_handler &rh) {
	return rh.reflect_base(*static_cast<flat_color_render_style *>(this));
}

void flat_color_renderer::set_texture(cgv::render::context &ctx, const cgv::render::texture &t) {
	has_texture = true;
	texture = t;
}

void flat_color_renderer::draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips,
							   bool use_adjacency, uint32_t strip_restart_index) {
	draw_impl(ctx, cgv::render::PT_TRIANGLES, start, count, use_strips, use_adjacency, strip_restart_index);
}

cgv::reflect::extern_reflection_traits<flat_color_render_style, flat_color_render_style_reflect>
get_reflection_traits(const flat_color_render_style &) {
	return cgv::reflect::extern_reflection_traits<flat_color_render_style, flat_color_render_style_reflect>();
}

#include <cgv/gui/provider.h>

struct flat_color_render_style_gui_creator : public cgv::gui::gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider *p, const std::string &label, void *value_ptr, const std::string &value_type,
				const std::string &gui_type, const std::string &options, bool *) override {
		if (value_type != cgv::type::info::type_name<flat_color_render_style>::get_name())
			return false;
		auto *strs_ptr = reinterpret_cast<flat_color_render_style *>(value_ptr);
		p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style *>(strs_ptr));
		return true;
	}
};

cgv::gui::gui_creator_registration<flat_color_render_style_gui_creator>
	  flat_color_rs_gc_reg("flat_color_render_style_gui_creator");
