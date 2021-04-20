#include "vr_env.h"

#include "deferred_renderer.h"
#include "simple_renderer.h"

vr_env::vr_env() { set_name("vr_env"); }

void vr_env::stream_help(std::ostream &os) {
	os << "Generates a default background for 3D visualizations in virtual reality" << std::endl;
}

bool vr_env::init(cgv::render::context &ctx) {
	ref_simple_renderer(ctx, 1);
	ref_deferred_renderer(ctx, 1);

	return true;
}

void vr_env::clear(cgv::render::context &ctx) {
	ref_simple_renderer(ctx, -1);
	ref_deferred_renderer(ctx, -1);
}

void vr_env::init_frame(cgv::render::context &ctx) { drawable::init_frame(ctx); }

void vr_env::draw(cgv::render::context &ctx) {
	{
		auto &simple = ref_simple_renderer(ctx);
		std::vector<vec3> positions = {
			  {0.0, 0.0, 0.0},
			  {1.0, 0.0, 0.0},
			  {1.0, 1.0, 0.0},
		};
		simple.set_position_array(ctx, positions);
		std::vector<vec3> colors = {
			  {0.0, 1.0, 1.0},
			  {1.0, 0.0, 1.0},
			  {1.0, 1.0, 0.0},
		};
		simple.set_color_array(ctx, colors);
		std::vector<unsigned int> indices = {0, 1, 2};
		simple.set_indices(ctx, indices);
		simple.render(ctx, 0, 3);
	}

	{
		auto &deferred = ref_deferred_renderer(ctx);
		std::vector<vec3> positions = {
			  {0.0, 0.0, 0.0},
			  {1.0, 0.0, 0.0},
			  {1.0, 1.0, 0.0},
			  {0.0, 1.0, 0.0},
		};
		deferred.set_position_array(ctx, positions);
		std::vector<vec2> texcoords = {
			  {0.0, 0.0},
			  {1.0, 0.0},
			  {1.0, 1.0},
			  {0.0, 1.0},
		};
		deferred.set_texcoord_array(ctx, texcoords);
		std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};
		deferred.set_indices(ctx, indices);
		deferred.render(ctx, 0, 6);
	}
}

void vr_env::finish_draw(cgv::render::context &ctx) { drawable::finish_draw(ctx); }

void vr_env::create_gui() {}

bool vr_env::handle(cgv::gui::event &e) { return false; }

std::string vr_env::get_type_name() const { return "vr_env"; }

cgv::base::object_registration<vr_env> vr_env_reg("vr_env"); // NOLINT(cert-err58-cpp)
