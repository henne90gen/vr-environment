#include "ssao_renderer.h"

#include <random>

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
	if (!surface_renderer::validate_attributes(ctx)) {
		return false;
	}

	if (gPosition == nullptr) {
		std::cerr << "gPosition cannot be a nullptr" << std::endl;
		return false;
	}

	if (gNormal == nullptr) {
		std::cerr << "gNormal cannot be a nullptr" << std::endl;
		return false;
	}

	return true;
}

float lerp(float a, float b, float f) { return a + f * (b - a); }

bool ssao_renderer::init(cgv::render::context &ctx) {
	bool res = surface_renderer::init(ctx);
	if (!ref_prog().is_created()) {
		if (!ref_prog().build_program(ctx, "ssao.glpr", true)) {
			std::cerr << "ERROR in ssao_renderer::init() ... could not build program ssao.glpr" << std::endl;
			return false;
		}
	}
	if (!res) {
		return false;
	}

	kernel.clear();
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;
	for (unsigned int i = 0; i < 64; ++i) {
		vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample.normalize();
		sample *= randomFloats(generator);
		float scale = float(i) / 64.0F;
		// scale samples so that they're more aligned to center of kernel
		scale = lerp(0.1F, 1.0F, scale * scale);
		sample *= scale;
		kernel.push_back(sample);
	}

	int width = 4;
	int height = 4;
	noise = cgv::render::texture(                                                       //
		  "flt32[R,G,B](" + std::to_string(width) + "," + std::to_string(height) + ")", //
		  cgv::render::TF_NEAREST, cgv::render::TF_NEAREST,                             //
		  cgv::render::TW_REPEAT, cgv::render::TW_REPEAT, cgv::render::TW_REPEAT        //
	);
	if (!noise.create(ctx, cgv::render::TT_2D, width, height)) {
		std::cerr << "Failed to create noise texture: " << noise.last_error << std::endl;
		return false;
	}

	std::vector<vec3> noiseData;
	for (int i = 0; i < width * height; i++) {
		noiseData.emplace_back(                      //
			  randomFloats(generator) * 2.0F - 1.0F, //
			  randomFloats(generator) * 2.0F - 1.0F, //
			  0.0F                                   //
		);
	}
	if (!noise.enable(ctx)) {
		std::cerr << "Failed to enable noise texture: " << noise.last_error << std::endl;
		return false;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, noiseData.data());

	return true;
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

	if (!noise.enable(ctx, 0)) {
		std::cerr << "Failed to enable noise texture: " << noise.last_error << std::endl;
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "texNoise", 0)) {
		return false;
	}

	if (!gPosition->enable(ctx, 1)) {
		std::cerr << "Failed to enable position texture: " << gPosition->last_error << std::endl;
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "gPosition", 1)) {
		return false;
	}

	if (!gNormal->enable(ctx, 2)) {
		std::cerr << "Failed to enable normal texture: " << gNormal->last_error << std::endl;
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "gNormal", 2)) {
		return false;
	}
	if (!fb.is_created()) {
		if (!fb.create(ctx)) {
			std::cerr << "Failed to create ssao framebuffer: " << fb.last_error << std::endl;
			return false;
		}

		unsigned int width = gPosition->get_width();
		unsigned int height = gPosition->get_height();
		auto w = std::to_string(width);
		auto h = std::to_string(height);
		*ssao_texture =
			  cgv::render::texture("flt32[R](" + w + "," + h + ")", cgv::render::TF_NEAREST, cgv::render::TF_NEAREST);
		if (!ssao_texture->create(ctx, cgv::render::TT_2D, width, height)) {
			std::cerr << "Failed to create ssao texture: " << ssao_texture->last_error << std::endl;
			return false;
		}
		if (!fb.attach(ctx, *ssao_texture)) {
			std::cerr << "Failed to attach ssao texture to ssao framebuffer: " << fb.last_error << std::endl;
			return false;
		}

		if (!fb.is_complete(ctx)) {
			std::cerr << "SSAO framebuffer is not complete: " << fb.last_error << std::endl;
			return false;
		}
	}
	if (!fb.enable(ctx)) {
		std::cerr << "Failed to enable ssao framebuffer: " << fb.last_error << std::endl;
		return false;
	}

	for (unsigned int i = 0; i < kernel.size(); ++i) {
		if (!ref_prog().set_uniform(ctx, "samples[" + std::to_string(i) + "]", kernel[i])) {
			std::cerr << "Failed to set kernel value " << i << std::endl;
			return false;
		}
	}
	if (!ref_prog().set_uniform(ctx, "screenWidth", static_cast<float>(ctx.get_width()))) {
		std::cerr << "Failed to set screen width" << std::endl;
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "screenHeight", static_cast<float>(ctx.get_height()))) {
		std::cerr << "Failed to set screen height" << std::endl;
		return false;
	}

	return true;
}

bool ssao_renderer::disable(cgv::render::context &ctx) {
	if (!surface_renderer::disable(ctx)) {
		return false;
	}

	glEnable(GL_BLEND);

	if (!fb.disable(ctx)) {
		std::cerr << "Failed to disable ssao framebuffer: " << fb.last_error << std::endl;
		return false;
	}

	return true;
}

bool ssao_render_style_reflect::self_reflect(cgv::reflect::reflection_handler &rh) {
	return rh.reflect_base(*static_cast<ssao_render_style *>(this));
}

void ssao_renderer::draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
						 uint32_t strip_restart_index) {
	draw_impl(ctx, cgv::render::PT_TRIANGLES, start, count, use_strips, use_adjacency, strip_restart_index);
}

void ssao_renderer::render(cgv::render::context &ctx, cgv::render::texture &_gPosition, cgv::render::texture &_gNormal,
						   cgv::render::texture &_ssao_texture) {
	this->gPosition = &_gPosition;
	this->gNormal = &_gNormal;
	this->ssao_texture = &_ssao_texture;
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
