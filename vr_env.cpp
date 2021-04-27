#include "vr_env.h"

#include <cgv_gl/box_renderer.h>

vr_env::vr_env() { set_name("vr_env"); }

void vr_env::stream_help(std::ostream &os) {
	os << "Generates a default background for 3D visualizations in virtual reality" << std::endl;
}

bool vr_env::init(cgv::render::context &ctx) {
	auto &deferred = ref_deferred_renderer(ctx, 1);
	if (!deferred.init(ctx)) {
		return false;
	}

	auto &simple = ref_flat_color_renderer(ctx, 1);
	if (!simple.init(ctx)) {
		return false;
	}

	if (!trees.init(ctx)) {
		return false;
	}

	return true;
}

void vr_env::clear(cgv::render::context &ctx) {
	ref_flat_color_renderer(ctx, -1);
	ref_deferred_renderer(ctx, -1);
	trees.clear(ctx);
}

void vr_env::init_frame(cgv::render::context &ctx) { drawable::init_frame(ctx); }

void vr_env::draw(cgv::render::context &ctx) {
	auto &deferred = ref_deferred_renderer(ctx);
	deferred.set_render_style(deferred_style);
	deferred.render(ctx, [&]() {
		ShaderToggles shaderToggles = {};
		TerrainParams terrainParams = {};
		trees.render(ctx, shaderToggles, terrainParams);

		auto &flat_color = ref_flat_color_renderer(ctx);
		flat_color.set_render_style(flat_color_style);
		flat_color.set_position_array(ctx, positions);
		flat_color.set_indices(ctx, indices);
		flat_color.render(ctx, 0, indices.size());
	});
}

void vr_env::finish_draw(cgv::render::context &ctx) { drawable::finish_draw(ctx); }

void vr_env::create_gui() {
	add_decorator("VR Environment", "heading", "level=2");

	if (begin_tree_node("deferred style", deferred_style)) {
		align("\a");
		add_gui("deferred style", deferred_style);
		align("\b");
		end_tree_node(deferred_style);
	}

	if (begin_tree_node("flat color style", flat_color_style)) {
		align("\a");
		add_gui("flat color style", flat_color_style);
		align("\b");
		end_tree_node(flat_color_style);
	}

	if (begin_tree_node("trees", trees)) {
		align("\a");
		add_gui("Trees", trees);
		align("\b");
		end_tree_node(trees);
	}
}

bool vr_env::handle(cgv::gui::event &e) { return false; }

std::string vr_env::get_type_name() const { return "vr_env"; }

void vr_env::on_set(void *member_ptr) {
	// NOTE: on_set is called every time something in the gui changes, thus we have to post a redraw event to make
	// sure any changes to the render styles ar being applied
	update_member(member_ptr);
	post_redraw();
	std::cout << "redraw" << std::endl;
}

cgv::base::object_registration<vr_env> vr_env_reg("vr_env"); // NOLINT(cert-err58-cpp)
