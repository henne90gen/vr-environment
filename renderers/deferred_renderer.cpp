#include "deferred_renderer.h"

#include <cgv_gl/gl/gl_tools.h>

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

	gBuffer = cgv::render::frame_buffer();
	if (!gBuffer.create(ctx)) {
		std::cerr << "Failed to create gBuffer: " << gBuffer.last_error << std::endl;
		return false;
	}

	const std::string ws = std::to_string(ctx.get_width());
	const std::string hs = std::to_string(ctx.get_height());
	const std::string description = std::string("flt16[R,G,B,A](") + ws + "," + hs + ")";
	gPosition = cgv::render::texture(    //
		  description,                   //
		  cgv::render::TF_NEAREST,       //
		  cgv::render::TF_NEAREST,       //
		  cgv::render::TW_CLAMP_TO_EDGE, //
		  cgv::render::TW_CLAMP_TO_EDGE, //
		  cgv::render::TW_CLAMP_TO_EDGE  //
	);
	if (!gPosition.create(ctx, cgv::render::TT_2D)) {
		std::cerr << "Failed to create position texture:" << gPosition.last_error << std::endl;
		return false;
	}
	if (!gBuffer.attach(ctx, gPosition, 0, 0)) {
		std::cerr << "Failed to attach position texture: " << gBuffer.last_error << std::endl;
		return false;
	}

	gNormal = cgv::render::texture(      //
		  description,                   //
		  cgv::render::TF_NEAREST,       //
		  cgv::render::TF_NEAREST,       //
		  cgv::render::TW_CLAMP_TO_EDGE, //
		  cgv::render::TW_CLAMP_TO_EDGE, //
		  cgv::render::TW_CLAMP_TO_EDGE  //
	);
	if (!gNormal.create(ctx, cgv::render::TT_2D)) {
		std::cerr << "Failed to create normal texture:" << gNormal.last_error << std::endl;
		return false;
	}
	if (!gBuffer.attach(ctx, gNormal, 0, 1)) {
		std::cerr << "Failed to attach normal texture: " << gBuffer.last_error << std::endl;
		return false;
	}

	const std::string albedoDescription = std::string("uint8[R,G,B,A](") + ws + "," + hs + ")";
	gAlbedo = cgv::render::texture(      //
		  albedoDescription,             //
		  cgv::render::TF_NEAREST,       //
		  cgv::render::TF_NEAREST,       //
		  cgv::render::TW_CLAMP_TO_EDGE, //
		  cgv::render::TW_CLAMP_TO_EDGE, //
		  cgv::render::TW_CLAMP_TO_EDGE  //
	);
	if (!gAlbedo.create(ctx, cgv::render::TT_2D)) {
		std::cerr << "Failed to create albedo texture:" << gAlbedo.last_error << std::endl;
		return false;
	}
	if (!gBuffer.attach(ctx, gAlbedo, 0, 2)) {
		std::cerr << "Failed to attach albedo texture: " << gBuffer.last_error << std::endl;
		return false;
	}

	// TODO also attach the doLightingBuffer
	// create do-lighting buffer
	//	GL_Call(glGenTextures(1, &gDoLighting));
	//	GL_Call(glBindTexture(GL_TEXTURE_2D, gDoLighting));
	//	GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr));
	//	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	//	GL_Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gDoLighting, 0));

	gDepth = cgv::render::render_buffer("[D]");
	if (!gDepth.create(ctx)) {
		std::cerr << "Failed to create depth buffer" << gDepth.last_error << std::endl;
		return false;
	}
	if (!gBuffer.attach(ctx, gDepth)) {
		std::cerr << "Failed to attach depth buffer: " << gBuffer.last_error << std::endl;
		return false;
	}

	if (!gBuffer.is_complete(ctx)) {
		std::cerr << "Failed to create framebuffer: " << gBuffer.last_error << std::endl;
		return false;
	}

	return true;
}

bool deferred_renderer::enable(cgv::render::context &ctx) {
	const auto &strs = get_style<deferred_render_style>();

	if (!surface_renderer::enable(ctx)) {
		return false;
	}

	if (!ref_prog().is_linked()) {
		return false;
	}

	if (!gPosition.enable(ctx, 0)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "gPosition", 0)) {
		return false;
	}

	if (!gNormal.enable(ctx, 1)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "gNormal", 1)) {
		return false;
	}

	if (!gAlbedo.enable(ctx, 2)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "gAlbedo", 2)) {
		return false;
	}

	const auto &style = get_style<deferred_render_style>();
	if (!ref_prog().set_uniform(ctx, "render_target", static_cast<int>(style.render_target))) {
		return false;
	}

	return true;
}

void deferred_renderer::render(cgv::render::context &ctx, const std::function<void()> &func) {
	if (!gBuffer.enable(ctx)) {
		std::cerr << "Failed to enable gBuffer: " << gBuffer.last_error << std::endl;
		return;
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	func();

	if (!gBuffer.disable(ctx)) {
		std::cerr << "Failed to disable gBuffer: " << gBuffer.last_error << std::endl;
		return;
	}

	set_position_array(ctx, positions);
	set_texcoord_array(ctx, texcoords);
	set_indices(ctx, indices);
	renderer::render(ctx, 0, indices.size());
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
		auto *style = reinterpret_cast<deferred_render_style *>(value_ptr);
		auto *b = dynamic_cast<cgv::base::base *>(p);
		p->add_member_control(
			  b, "render target", style->render_target, "dropdown",
			  "enums='DEFAULT=0,POSITION=1,NORMAL=2,ALBEDO=3';tooltip='The final texture to draw to the screen.'");
		return true;
	}
};

cgv::gui::gui_creator_registration<deferred_render_style_gui_creator>
	  deferred_rs_gc_reg("deferred_render_style_gui_creator");
