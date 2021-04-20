#include "deferred_renderer.h"

#include <cgv_gl/gl/gl_tools.h>

#include <array>

deferred_renderer &ref_deferred_renderer(cgv::render::context &ctx, int ref_count_change) {
	static int ref_count = 0;
	static deferred_renderer r;
	r.manage_singelton(ctx, "deferred_renderer", ref_count, ref_count_change);
	return r;
}

cgv::render::render_style *deferred_renderer::create_render_style() const { return new deferred_render_style(); }

bool deferred_renderer::validate_attributes(const cgv::render::context &ctx) const {
	const auto &strs = get_style<deferred_render_style>();
	return surface_renderer::validate_attributes(ctx);
}

bool deferred_renderer::init(cgv::render::context &ctx) {
	bool res = surface_renderer::init(ctx);
	if (!ref_prog().is_created()) {
		if (!ref_prog().build_program(ctx, "deferred.glpr", true)) {
			std::cerr << "ERROR in deferred_renderer::init() ... could not build program deferred.glpr" << std::endl;
			return false;
		}
	}

	if (!res) {
		return false;
	}

	unsigned int width = ctx.get_width();
	unsigned int height = ctx.get_height();

	// Create framebuffer object
	GL_Call(glGenFramebuffers(1, &gBuffer));
	GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));

	// Create position buffer
	GL_Call(glGenTextures(1, &gPosition));
	GL_Call(glBindTexture(GL_TEXTURE_2D, gPosition));
	GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0));

	// Create normal buffer
	GL_Call(glGenTextures(1, &gNormal));
	GL_Call(glBindTexture(GL_TEXTURE_2D, gNormal));
	GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0));

	// create color buffer
	GL_Call(glGenTextures(1, &gAlbedo));
	GL_Call(glBindTexture(GL_TEXTURE_2D, gAlbedo));
	GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0));

	// create do-lighting buffer
	GL_Call(glGenTextures(1, &gDoLighting));
	GL_Call(glBindTexture(GL_TEXTURE_2D, gDoLighting));
	GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gDoLighting, 0));

	std::array<unsigned int, 4> attachments = {
		  GL_COLOR_ATTACHMENT0, //
		  GL_COLOR_ATTACHMENT1, //
		  GL_COLOR_ATTACHMENT2, //
		  GL_COLOR_ATTACHMENT3, //
	};
	glDrawBuffers(attachments.size(), reinterpret_cast<unsigned int *>(&attachments[0]));

	// Create depth buffer
	GL_Call(glGenRenderbuffers(1, &depthBuffer));
	GL_Call(glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer));
	GL_Call(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
	GL_Call(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer));

	res = validate_framebuffer();
	GL_Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	return res;
}

bool deferred_renderer::enable(cgv::render::context &ctx) {
	const auto &strs = get_style<deferred_render_style>();

	if (!surface_renderer::enable(ctx))
		return false;

	if (!ref_prog().is_linked())
		return false;

	return true;
}

bool deferred_renderer::disable(cgv::render::context &ctx) { return surface_renderer::disable(ctx); }

bool deferred_render_style_reflect::self_reflect(cgv::reflect::reflection_handler &rh) {
	return rh.reflect_base(*static_cast<deferred_render_style *>(this));
}

void deferred_renderer::draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
							 uint32_t strip_restart_index) {
	draw_impl(ctx, cgv::render::PT_TRIANGLES, start, count, use_strips, use_adjacency, strip_restart_index);
}

cgv::reflect::extern_reflection_traits<deferred_render_style, deferred_render_style_reflect>
get_reflection_traits(const deferred_render_style &) {
	return cgv::reflect::extern_reflection_traits<deferred_render_style, deferred_render_style_reflect>();
}

#include <cgv/gui/provider.h>

struct deferred_render_style_gui_creator : public cgv::gui::gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider *p, const std::string &label, void *value_ptr, const std::string &value_type,
				const std::string &gui_type, const std::string &options, bool *) override {
		if (value_type != cgv::type::info::type_name<deferred_render_style>::get_name())
			return false;
		auto *strs_ptr = reinterpret_cast<deferred_render_style *>(value_ptr);
		p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style *>(strs_ptr));
		return true;
	}
};

cgv::gui::gui_creator_registration<deferred_render_style_gui_creator>
	  deferred_rs_gc_reg("deferred_render_style_gui_creator");
