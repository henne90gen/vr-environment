#include "deferred_renderer.h"

#include <cgv_gl/gl/gl_tools.h>

#include "../macros.h"
#include "blur_renderer.h"
#include "ssao_renderer.h"

deferred_renderer &ref_deferred_renderer(cgv::render::context &ctx, int ref_count_change) {
	static int ref_count = 0;
	static deferred_renderer r;
	r.manage_singleton(ctx, "deferred_renderer", ref_count, ref_count_change);
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

	auto &ssao = ref_ssao_renderer(ctx, 1);
	if (!ssao.init(ctx)) {
		return false;
	}

	auto &blur = ref_blur_renderer(ctx, 1);
	if (!blur.init(ctx)) {
		return false;
	}

	return init_g_buffer(ctx);
}

bool deferred_renderer::init_g_buffer(cgv::render::context &ctx){
    gBuffer = cgv::render::frame_buffer();
    if (!gBuffer.create(ctx)) {
        std::cerr << "Failed to create gBuffer: " << gBuffer.last_error << std::endl;
        return false;
    }

    const std::string ws = std::to_string(ctx.get_width());
    const std::string hs = std::to_string(ctx.get_height());
    gPosition = cgv::render::texture(              //
          "flt16[R,G,B,A](" + ws + "," + hs + ")", //
          cgv::render::TF_NEAREST,                 //
          cgv::render::TF_NEAREST,                 //
          cgv::render::TW_CLAMP_TO_EDGE,           //
          cgv::render::TW_CLAMP_TO_EDGE,           //
          cgv::render::TW_CLAMP_TO_EDGE            //
    );
    if (!gPosition.create(ctx, cgv::render::TT_2D)) {
        std::cerr << "Failed to create position texture:" << gPosition.last_error << std::endl;
        return false;
    }
    if (!gBuffer.attach(ctx, gPosition, 0, 0)) {
        std::cerr << "Failed to attach position texture: " << gBuffer.last_error << std::endl;
        return false;
    }

    gNormal = cgv::render::texture(                //
          "flt16[R,G,B,A](" + ws + "," + hs + ")", //
          cgv::render::TF_NEAREST,                 //
          cgv::render::TF_NEAREST,                 //
          cgv::render::TW_CLAMP_TO_EDGE,           //
          cgv::render::TW_CLAMP_TO_EDGE,           //
          cgv::render::TW_CLAMP_TO_EDGE            //
    );
    if (!gNormal.create(ctx, cgv::render::TT_2D)) {
        std::cerr << "Failed to create normal texture:" << gNormal.last_error << std::endl;
        return false;
    }
    if (!gBuffer.attach(ctx, gNormal, 0, 1)) {
        std::cerr << "Failed to attach normal texture: " << gBuffer.last_error << std::endl;
        return false;
    }

    gAlbedo = cgv::render::texture(                //
          "uint8[R,G,B,A](" + ws + "," + hs + ")", //
          cgv::render::TF_NEAREST,                 //
          cgv::render::TF_NEAREST,                 //
          cgv::render::TW_CLAMP_TO_EDGE,           //
          cgv::render::TW_CLAMP_TO_EDGE,           //
          cgv::render::TW_CLAMP_TO_EDGE            //
    );
    if (!gAlbedo.create(ctx, cgv::render::TT_2D)) {
        std::cerr << "Failed to create albedo texture:" << gAlbedo.last_error << std::endl;
        return false;
    }
    if (!gBuffer.attach(ctx, gAlbedo, 0, 2)) {
        std::cerr << "Failed to attach albedo texture: " << gBuffer.last_error << std::endl;
        return false;
    }

    // TODO this can be optimized by reducing the texture to a single channel
    gIsCloud = cgv::render::texture(               //
          "uint8[R,G,B,A](" + ws + "," + hs + ")", //
          cgv::render::TF_NEAREST,                 //
          cgv::render::TF_NEAREST,                 //
          cgv::render::TW_CLAMP_TO_EDGE,           //
          cgv::render::TW_CLAMP_TO_EDGE,           //
          cgv::render::TW_CLAMP_TO_EDGE            //
    );
    if (!gIsCloud.create(ctx, cgv::render::TT_2D)) {
        std::cerr << "Failed to create is_cloud texture:" << gIsCloud.last_error << std::endl;
        return false;
    }
    if (!gBuffer.attach(ctx, gIsCloud, 0, 3)) {
        std::cerr << "Failed to attach is_cloud texture: " << gBuffer.last_error << std::endl;
        return false;
    }

    gDepth = cgv::render::texture(           //
          "flt32[D](" + ws + "," + hs + ")", //
          cgv::render::TF_NEAREST,           //
          cgv::render::TF_NEAREST,           //
          cgv::render::TW_CLAMP_TO_EDGE,     //
          cgv::render::TW_CLAMP_TO_EDGE,     //
          cgv::render::TW_CLAMP_TO_EDGE      //
    );
    gDepth.set_compare_mode(false);
    gDepth.set_compare_function(cgv::render::CF_LEQUAL);
    if (!gDepth.create(ctx, cgv::render::TT_2D)) {
        std::cerr << "Failed to create depth buffer: " << gDepth.last_error << std::endl;
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

	if (!gIsCloud.enable(ctx, 3)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "gIsCloud", 3)) {
		return false;
	}

	if (!ssao_texture.enable(ctx, 4)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "ssao", 4)) {
		return false;
	}

	if (!blurred_ssao_texture.enable(ctx, 5)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "ssaoBlur", 5)) {
		return false;
	}

	if (!gDepth.enable(ctx, 6)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "gDepth", 6)) {
		return false;
	}

	const auto &style = get_style<deferred_render_style>();
	if (!ref_prog().set_uniform(ctx, "render_target", static_cast<int>(style.render_target))) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "use_atmospheric_scattering", style.use_atmospheric_scattering)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "use_ambient_occlusion", style.use_ambient_occlusion)) {
		return false;
	}

	return true;
}

void deferred_renderer::render(cgv::render::context &ctx, const std::function<void()> &func) {
	if (gBuffer.get_width() != ctx.get_width() || gBuffer.get_height() != ctx.get_height()) {
		if (!init_g_buffer(ctx)) {
			std::cerr << "Failed to resize gBuffer" << std::endl;
		}
	}

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

	auto &ssao = ref_ssao_renderer(ctx);
	ssao.render(ctx, gPosition, gNormal, ssao_texture);

	auto &blur = ref_blur_renderer(ctx);
	blurred_ssao_texture.set_component_format(ssao_texture.get_component_format());
	blur.render(ctx, ssao_texture, blurred_ssao_texture);

	set_position_array(ctx, positions);
	set_texcoord_array(ctx, texcoords);
	set_indices(ctx, indices);
	renderer::render(ctx, 0, indices.size());

	{
		//  NOTE: copying the depth buffer from the gBuffer to the main depth buffer, because the depth information is
		//  later used by the stereo view to allow scene navigation
		GLint fbo_id;
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &fbo_id);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, get_gl_id(gBuffer.handle));
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
		int width = static_cast<int>(ctx.get_width());
		int height = static_cast<int>(ctx.get_height());
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id);
	}
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
			  "enums='DEFAULT=0,POSITION=1,NORMAL=2,ALBEDO=3,IS_CLOUD=4,SSAO=5,SSAO_BLUR=6,DEPTH=7,TEST=8';tooltip='"
			  "The final texture to "
			  "draw to the screen.'");
		p->add_member_control(b, "use atmospheric scattering", style->use_atmospheric_scattering);
		p->add_member_control(b, "use ambient occlusion", style->use_ambient_occlusion);
		p->add_member_control(b, "use aces film", style->use_aces_film);

		return true;
	}
};

cgv::gui::gui_creator_registration<deferred_render_style_gui_creator>
	  deferred_rs_gc_reg("deferred_render_style_gui_creator");
