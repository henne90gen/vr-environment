#include "vr_env.h"

#include "simple_renderer.h"

vr_env::vr_env() {}

void vr_env::stream_help(std::ostream &os) {
    os << "Generates a default background for 3D visualizations in virtual reality" << std::endl;
}

bool vr_env::init(cgv::render::context &ctx) {
    auto simple = ref_simple_renderer(ctx, 1);
    std::vector<vec3> positions = {
          {0.0, 0.0, 0.0},
          {1.0, 0.0, 0.0},
          {1.0, 1.0, 0.0},
    };
    simple.set_position_array(ctx, positions);
    std::vector<unsigned int> indices = {0, 1, 2};
    simple.set_indices(ctx, indices);
    return true;
}

void vr_env::clear(cgv::render::context &ctx) {
    auto simple = ref_simple_renderer(ctx, -1);
    simple.render(ctx, 0, 3);
}

void vr_env::init_frame(cgv::render::context &ctx) { drawable::init_frame(ctx); }

void vr_env::draw(cgv::render::context &ctx) {
    auto simple = ref_simple_renderer(ctx, 0);
    simple.render(ctx, 0, 3);
}

void vr_env::finish_draw(cgv::render::context &ctx) { drawable::finish_draw(ctx); }

void vr_env::create_gui() {}

bool vr_env::handle(cgv::gui::event &e) { return false; }

cgv::base::object_registration<vr_env> vr_env_reg("vr_env"); // NOLINT(cert-err58-cpp)
