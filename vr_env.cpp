#include "vr_env.h"

#include "deferred_renderer.h"
#include "simple_renderer.h"

vr_env::vr_env() { set_name("vr_env"); }

void vr_env::stream_help(std::ostream &os) {
	os << "Generates a default background for 3D visualizations in virtual reality" << std::endl;
}

bool vr_env::init(cgv::render::context &ctx) {
	auto &deferred = ref_deferred_renderer(ctx, 1);
	if (!deferred.init(ctx)) {
		return false;
	}

	auto &simple = ref_simple_renderer(ctx, 1);
	if (!simple.init(ctx)) {
		return false;
	}

	return true;
}

void vr_env::clear(cgv::render::context &ctx) {
	ref_simple_renderer(ctx, -1);
	ref_deferred_renderer(ctx, -1);
}

void vr_env::init_frame(cgv::render::context &ctx) { drawable::init_frame(ctx); }

void vr_env::draw(cgv::render::context &ctx) {
	auto &deferred = ref_deferred_renderer(ctx);

	deferred.render(ctx, [&]() {
		auto &simple = ref_simple_renderer(ctx);

		auto tex = cgv::render::texture("uint8[R,G,B,A]", cgv::render::TF_NEAREST, cgv::render::TF_NEAREST);
		const std::string fileName = "../../texture_test.bmp";
		if (!tex.create_from_image(ctx, fileName)) {
			std::cerr << "failed to create texture from image: " << tex.last_error << std::endl;
			return;
		}
		if (!tex.enable(ctx, 0)) {
			return;
		}
		simple.ref_prog().set_uniform(ctx, "tex", 0);

		simple.set_position_array(ctx, positions);
		simple.set_texcoord_array(ctx, texcoords);
		simple.set_indices(ctx, indices);
		simple.render(ctx, 0, indices.size());
	});
}

void vr_env::finish_draw(cgv::render::context &ctx) { drawable::finish_draw(ctx); }

void vr_env::create_gui() {}

bool vr_env::handle(cgv::gui::event &e) { return false; }

std::string vr_env::get_type_name() const { return "vr_env"; }

cgv::base::object_registration<vr_env> vr_env_reg("vr_env"); // NOLINT(cert-err58-cpp)
