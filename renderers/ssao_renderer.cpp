#include "ssao_renderer.h"

#include <cgv_gl/gl/gl_tools.h>

ssao_renderer &ref_ssao_renderer(cgv::render::context &ctx, int ref_count_change) {
	static int ref_count = 0;
	static ssao_renderer r;
	r.manage_singelton(ctx, "ssao_renderer", ref_count, ref_count_change);
	return r;
}

cgv::render::render_style *ssao_renderer::create_render_style() const { return new ssao_render_style(); }

bool ssao_renderer::validate_attributes(const cgv::render::context &ctx) const {
	const auto &strs = get_style<ssao_render_style>();
	return surface_renderer::validate_attributes(ctx);
}

bool ssao_renderer::init(cgv::render::context &ctx) {
	bool res = surface_renderer::init(ctx);
	if (!ref_prog().is_created()) {
		if (!ref_prog().build_program(ctx, "ssao.glpr", true)) {
			std::cerr << "ERROR in ssao_renderer::init() ... could not build program ssao.glpr" << std::endl;
			return false;
		}
	}
	return res;
}

bool ssao_renderer::enable(cgv::render::context &ctx) {
	const auto &strs = get_style<ssao_render_style>();

	if (!surface_renderer::enable(ctx)) {
		return false;
	}

	if (!ref_prog().is_linked()) {
		return false;
	}

	glDisable(GL_BLEND);

	return true;
}

bool ssao_renderer::disable(cgv::render::context &ctx) {
	if (!surface_renderer::disable(ctx)) {
		return false;
	}

	glEnable(GL_BLEND);
}

bool ssao_render_style_reflect::self_reflect(cgv::reflect::reflection_handler &rh) {
	return rh.reflect_base(*static_cast<ssao_render_style *>(this));
}

void ssao_renderer::draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
						 uint32_t strip_restart_index) {
	draw_impl(ctx, cgv::render::PT_TRIANGLES, start, count, use_strips, use_adjacency, strip_restart_index);
}

void ssao_renderer::render(cgv::render::context &ctx) {
	set_position_array(ctx, positions);
	set_texcoord_array(ctx, texcoords);
	set_indices(ctx, indices);
	renderer::render(ctx, 0, indices.size());
}

cgv::reflect::extern_reflection_traits<ssao_render_style, ssao_render_style_reflect>
get_reflection_traits(const ssao_render_style &) {
	return cgv::reflect::extern_reflection_traits<ssao_render_style, ssao_render_style_reflect>();
}

#include <cgv/gui/provider.h>

struct ssao_render_style_gui_creator : public cgv::gui::gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider *p, const std::string &label, void *value_ptr, const std::string &value_type,
				const std::string &gui_type, const std::string &options, bool *) override {
		if (value_type != cgv::type::info::type_name<ssao_render_style>::get_name())
			return false;
		return true;
	}
};

cgv::gui::gui_creator_registration<ssao_render_style_gui_creator> ssao_rs_gc_reg("ssao_render_style_gui_creator");
