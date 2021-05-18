#include "clouds_renderer.h"

#include <cgv_gl/gl/gl_tools.h>

clouds_renderer &ref_clouds_renderer(cgv::render::context &ctx, int ref_count_change) {
	static int ref_count = 0;
	static clouds_renderer r;
	r.manage_singleton(ctx, "clouds_renderer", ref_count, ref_count_change);
	return r;
}

cgv::render::render_style *clouds_renderer::create_render_style() const { return new clouds_render_style(); }

bool clouds_renderer::validate_attributes(const cgv::render::context &ctx) const {
	const auto &strs = get_style<clouds_render_style>();
	return surface_renderer::validate_attributes(ctx);
}

bool clouds_renderer::init(cgv::render::context &ctx) {
	bool res = surface_renderer::init(ctx);
	if (!ref_prog().is_created()) {
		if (!ref_prog().build_program(ctx, "clouds.glpr", true)) {
			std::cerr << "ERROR in clouds_renderer::init() ... could not build program clouds.glpr" << std::endl;
			return false;
		}
	}
	return res;
}

bool clouds_renderer::enable(cgv::render::context &ctx) {
	const auto &strs = get_style<clouds_render_style>();

	if (!surface_renderer::enable(ctx)) {
		return false;
	}

	if (!ref_prog().is_linked()) {
		return false;
	}

	const auto &style = get_style<clouds_render_style>();
	if (!ref_prog().set_uniform(ctx, "cloud_blend", style.cloud_blend)) {
		return false;
	}

	if (!ref_prog().set_uniform(ctx, "animation_speed", style.animation_speed)) {
		return false;
	}

	animation_time += 0.01F;
	if (!ref_prog().set_uniform(ctx, "animation_time", animation_time)) {
		return false;
	}

	// FIXME find a better way to get this in to the shader (maybe through the model matrix somehow?)
	if (!ref_prog().set_uniform(ctx, "clouds_position", style.clouds_position)) {
		return false;
	}

	if (!ref_prog().set_uniform(ctx, "clouds_scale", style.clouds_scale)) {
		return false;
	}

	return true;
}

bool clouds_renderer::disable(cgv::render::context &ctx) { return surface_renderer::disable(ctx); }

bool clouds_render_style_reflect::self_reflect(cgv::reflect::reflection_handler &rh) {
	return rh.reflect_base(*static_cast<clouds_render_style *>(this));
}

void clouds_renderer::draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
						   uint32_t strip_restart_index) {
	draw_impl(ctx, cgv::render::PT_TRIANGLES, start, count, use_strips, use_adjacency, strip_restart_index);
}

void clouds_renderer::render(cgv::render::context &ctx) {
	const auto &style = get_style<clouds_render_style>();
	if (!style.enabled) {
		return;
	}

	set_position_array(ctx, positions);
	set_texcoord_array(ctx, texcoords);
	set_indices(ctx, indices);
	renderer::render(ctx, 0, indices.size());
}

cgv::reflect::extern_reflection_traits<clouds_render_style, clouds_render_style_reflect>
get_reflection_traits(const clouds_render_style &) {
	return cgv::reflect::extern_reflection_traits<clouds_render_style, clouds_render_style_reflect>();
}

#include <cgv/gui/provider.h>

struct clouds_render_style_gui_creator : public cgv::gui::gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider *p, const std::string &label, void *value_ptr, const std::string &value_type,
				const std::string &gui_type, const std::string &options, bool *) override {
		if (value_type != cgv::type::info::type_name<clouds_render_style>::get_name())
			return false;

		auto *style = reinterpret_cast<clouds_render_style *>(value_ptr);
		auto *b = dynamic_cast<cgv::base::base *>(p);
		p->add_member_control(b, "Enabled", style->enabled);
		p->add_member_control(b, "Cloud Blend", style->cloud_blend);
		p->add_member_control(b, "Animation Speed", style->animation_speed);

		return true;
	}
};

cgv::gui::gui_creator_registration<clouds_render_style_gui_creator>
	  clouds_rs_gc_reg("clouds_render_style_gui_creator");
