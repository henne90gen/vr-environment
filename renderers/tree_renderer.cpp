#include "tree_renderer.h"

#include <cgv_gl/gl/gl_tools.h>

tree_renderer &ref_tree_renderer(cgv::render::context &ctx, int ref_count_change) {
	static int ref_count = 0;
	static tree_renderer r;
	r.manage_singleton(ctx, "tree_renderer", ref_count, ref_count_change);
	return r;
}

cgv::render::render_style *tree_renderer::create_render_style() const { return new tree_render_style(); }

bool tree_renderer::validate_attributes(const cgv::render::context &ctx) const {
	const auto &strs = get_style<tree_render_style>();
	return surface_renderer::validate_attributes(ctx);
}

bool tree_renderer::init(cgv::render::context &ctx) {
	bool res = surface_renderer::init(ctx);
	if (!ref_prog().is_created()) {
		if (!ref_prog().build_program(ctx, "trees.glpr", true)) {
			std::cerr << "ERROR in tree_renderer::init() ... could not build program trees.glpr" << std::endl;
			return false;
		}
	}

	if (!surface_texture.create_from_image(ctx, "../../assets/textures/bark_and_leafs_light.png")) {
		std::cerr << "Failed to load tree surface texture: " << surface_texture.last_error << std::endl;
		has_surface_texture = false;
	} else {
		has_surface_texture = true;
	}

	return res;
}

bool tree_renderer::enable(cgv::render::context &ctx) {
	const auto &strs = get_style<tree_render_style>();

	if (!surface_renderer::enable(ctx)) {
		return false;
	}

	if (!ref_prog().is_linked()) {
		return false;
	}

	const auto &style = get_style<tree_render_style>();
	if (!ref_prog().set_uniform(ctx, "flat_color", style.surface_color)) {
		return false;
	}

	if (!ref_prog().set_uniform(ctx, "has_position_texture", has_position_texture)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "position_texture", 0)) {
		return false;
	}
	if (!position_texture.enable(ctx, 0)) {
		return false;
	}

	if (!ref_prog().set_uniform(ctx, "has_surface_texture", has_surface_texture)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "surface_texture", 1)) {
		return false;
	}
	if (!surface_texture.enable(ctx, 1)) {
		return false;
	}

	if (!ref_prog().set_uniform(ctx, "tree_count", style.tree_count)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "rotation", style.rotation)) {
		return false;
	}

	return true;
}

bool tree_renderer::disable(cgv::render::context &ctx) { return surface_renderer::disable(ctx); }

bool tree_render_style_reflect::self_reflect(cgv::reflect::reflection_handler &rh) {
	return rh.reflect_base(*static_cast<tree_render_style *>(this));
}

void tree_renderer::set_position_texture(cgv::render::context &ctx, const cgv::render::texture &t) {
	has_position_texture = true;
	position_texture = t;
}

void tree_renderer::set_surface_texture(cgv::render::context &ctx, const cgv::render::texture &t) {
	has_surface_texture = true;
	surface_texture = t;
}

void tree_renderer::draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
						 uint32_t strip_restart_index) {
	const auto &style = get_style<tree_render_style>();
	draw_impl_instanced(ctx, cgv::render::PT_TRIANGLES, start, count, style.tree_count, use_strips, use_adjacency,
						strip_restart_index);
}

cgv::reflect::extern_reflection_traits<tree_render_style, tree_render_style_reflect>
get_reflection_traits(const tree_render_style &) {
	return cgv::reflect::extern_reflection_traits<tree_render_style, tree_render_style_reflect>();
}

#include <cgv/gui/provider.h>

struct tree_render_style_gui_creator : public cgv::gui::gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider *p, const std::string &label, void *value_ptr, const std::string &value_type,
				const std::string &gui_type, const std::string &options, bool *) override {
		if (value_type != cgv::type::info::type_name<tree_render_style>::get_name())
			return false;
		auto *style = reinterpret_cast<tree_render_style *>(value_ptr);
		p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style *>(style));

		auto *b = dynamic_cast<cgv::base::base *>(p);
		p->add_member_control(b, "Tree Count", style->tree_count);
		p->add_member_control(b, "Tree Rotation", style->rotation);
		return true;
	}
};

cgv::gui::gui_creator_registration<tree_render_style_gui_creator> tree_rs_gc_reg("tree_render_style_gui_creator");
