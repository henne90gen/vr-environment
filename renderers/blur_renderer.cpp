#include "blur_renderer.h"

#include <cgv_gl/gl/gl_tools.h>

blur_renderer &ref_blur_renderer(cgv::render::context &ctx, int ref_count_change) {
	static int ref_count = 0;
	static blur_renderer r;
	r.manage_singleton(ctx, "blur_renderer", ref_count, ref_count_change);
	return r;
}

cgv::render::render_style *blur_renderer::create_render_style() const { return new blur_render_style(); }

bool blur_renderer::validate_attributes(const cgv::render::context &ctx) const {
	const auto &strs = get_style<blur_render_style>();
	if (!surface_renderer::validate_attributes(ctx)) {
		return false;
	}

	if (texture_to_be_blurred == nullptr) {
		std::cerr << "Texture to be blurred cannot be a nullptr" << std::endl;
		return false;
	}

	if (blurred_texture == nullptr) {
		std::cerr << "Blurred texture cannot be a nullptr" << std::endl;
		return false;
	}

	return true;
}

bool blur_renderer::init(cgv::render::context &ctx) {
	bool res = surface_renderer::init(ctx);
	if (!ref_prog().is_created()) {
		if (!ref_prog().build_program(ctx, "blur.glpr", true)) {
			std::cerr << "ERROR in blur_renderer::init() ... could not build program blur.glpr" << std::endl;
			return false;
		}
	}
	if (!res) {
		return false;
	}

	return true;
}

bool blur_renderer::enable(cgv::render::context &ctx) {
	const auto &strs = get_style<blur_render_style>();

	if (!surface_renderer::enable(ctx)) {
		return false;
	}

	if (!ref_prog().is_linked()) {
		return false;
	}

	if (!fb.is_created() || fb.get_width() != ctx.get_width() || fb.get_height() != ctx.get_height()) {
		if (!fb.create(ctx)) {
			std::cerr << "Failed to create ssao framebuffer: " << fb.last_error << std::endl;
			return false;
		}

		unsigned int width = texture_to_be_blurred->get_width();
		unsigned int height = texture_to_be_blurred->get_height();
		if (!blurred_texture->create(ctx, cgv::render::TT_2D, width, height)) {
			std::cerr << "Failed to create blurred texture: " << blurred_texture->last_error << std::endl;
			return false;
		}
		if (!fb.attach(ctx, *blurred_texture)) {
			std::cerr << "Failed to attach blurred texture to blur framebuffer: " << fb.last_error << std::endl;
			return false;
		}

		if (!fb.is_complete(ctx)) {
			std::cerr << "Blur framebuffer is not complete: " << fb.last_error << std::endl;
			return false;
		}
	}

	if (!fb.enable(ctx)) {
		std::cerr << "Failed to enable blur framebuffer: " << fb.last_error << std::endl;
		return false;
	}

	if (!ref_prog().set_uniform(ctx, "texture_sampler", 0)) {
		return false;
	}
	if (!texture_to_be_blurred->enable(ctx, 0)) {
		return false;
	}

	return true;
}

bool blur_renderer::disable(cgv::render::context &ctx) {
	if (!surface_renderer::disable(ctx)) {
		return false;
	}

	if (!fb.disable(ctx)) {
		std::cerr << "Failed to disable blur framebuffer: " << fb.last_error << std::endl;
		return false;
	}

	return true;
}

bool blur_render_style_reflect::self_reflect(cgv::reflect::reflection_handler &rh) {
	return rh.reflect_base(*static_cast<blur_render_style *>(this));
}

void blur_renderer::draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
						 uint32_t strip_restart_index) {
	draw_impl(ctx, cgv::render::PT_TRIANGLES, start, count, use_strips, use_adjacency, strip_restart_index);
}

void blur_renderer::render(cgv::render::context &ctx, cgv::render::texture &_texture_to_be_blurred,
						   cgv::render::texture &_blurred_texture) {
	this->texture_to_be_blurred = &_texture_to_be_blurred;
	this->blurred_texture = &_blurred_texture;
	set_position_array(ctx, positions);
	set_texcoord_array(ctx, texcoords);
	set_indices(ctx, indices);
	renderer::render(ctx, 0, indices.size());
}

cgv::reflect::extern_reflection_traits<blur_render_style, blur_render_style_reflect>
get_reflection_traits(const blur_render_style &) {
	return cgv::reflect::extern_reflection_traits<blur_render_style, blur_render_style_reflect>();
}

#include <cgv/gui/provider.h>

struct blur_render_style_gui_creator : public cgv::gui::gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider *p, const std::string &label, void *value_ptr, const std::string &value_type,
				const std::string &gui_type, const std::string &options, bool *) override {
		if (value_type != cgv::type::info::type_name<blur_render_style>::get_name())
			return false;
		auto *strs_ptr = reinterpret_cast<blur_render_style *>(value_ptr);
		p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style *>(strs_ptr));
		return true;
	}
};

cgv::gui::gui_creator_registration<blur_render_style_gui_creator> blur_rs_gc_reg("blur_render_style_gui_creator");
