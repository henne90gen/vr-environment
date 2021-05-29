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

	auto &flat_color = ref_flat_color_renderer(ctx, 1);
	if (!flat_color.init(ctx)) {
		return false;
	}

	auto &box_renderer = cgv::render::ref_box_renderer(ctx, 1);
	if (!box_renderer.init(ctx)) {
		return false;
	}

	auto &clouds_renderer = ref_clouds_renderer(ctx, 1);
	if (!clouds_renderer.init(ctx)) {
		return false;
	}

	auto &terrain_renderer = ref_terrain_renderer(ctx, 1);
	if (!terrain_renderer.init(ctx)) {
		return false;
	}

	test_texture = cgv::render::texture();
	if (!test_texture.create_from_image(ctx, "../../texture_test.bmp")) {
		std::cerr << "failed to create test texture: " << test_texture.last_error << std::endl;
		return false;
	}

	if (!trees.init(ctx)) {
		return false;
	}

	return true;
}

void vr_env::clear(cgv::render::context &ctx) {
	ref_flat_color_renderer(ctx, -1);
	cgv::render::ref_box_renderer(ctx, -1);
	ref_deferred_renderer(ctx, -1);
	ref_clouds_renderer(ctx, -1);
	ref_terrain_renderer(ctx, -1);
	trees.clear(ctx);
}

void vr_env::init_frame(cgv::render::context &ctx) { drawable::init_frame(ctx); }

void vr_env::draw(cgv::render::context &ctx) {
	auto &deferred = ref_deferred_renderer(ctx);
	deferred.set_render_style(deferred_style);
	deferred.render(ctx, [&]() {
		auto &terrain = ref_terrain_renderer(ctx);
		terrain.set_render_style(terrain_style);
		terrain.render(ctx, terrainParams);

		trees.render(ctx, shaderToggles, terrainParams);

		auto &clouds = ref_clouds_renderer(ctx);
		clouds.set_render_style(clouds_style);
		clouds.render(ctx);
	});

	std::vector<box3> boxes = {};
	for (int i = 0; i < 100; i++) {
		boxes.emplace_back(vec3(i - 1), vec3(i + 1));
	}

	auto &box_renderer = cgv::render::ref_box_renderer(ctx, 1);
	box_renderer.set_box_array(ctx, boxes);
	box_renderer.render(ctx, 0, boxes.size());
}

void vr_env::finish_draw(cgv::render::context &ctx) { drawable::finish_draw(ctx); }

void vr_env::create_gui() {
	add_decorator("VR Environment", "heading", "level=2");

	if (begin_tree_node("Deferred Rendering", deferred_style)) {
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

	if (begin_tree_node("Terrain", terrain_style)) {
		align("\a");
		add_gui("terrain style", terrain_style);
		align("\b");
		end_tree_node(terrain_style);
	}

	if (begin_tree_node("Clouds", clouds_style)) {
		align("\a");
		add_gui("clouds", clouds_style);
		align("\b");
		end_tree_node(clouds_style);
	}

	if (begin_tree_node("Trees", trees)) {
		align("\a");
		add_gui("Trees", trees);
		align("\b");
		end_tree_node(trees);
	}

	add_member_control(this, "Seed", terrainParams.seed);
	connect_copy(add_button("New Seed")->click, cgv::signal::rebind(this, &vr_env::generate_new_seed));
	add_member_control(this, "Steepness", terrainParams.power);
	add_member_control(this, "Bowl Strength", terrainParams.bowlStrength);
	add_member_control(this, "Platform Height", terrainParams.platformHeight);

	if (begin_tree_node_void("Noise Layers", this)) {
		align("\a");
		for (int i = 0; i < static_cast<int>(terrainParams.noiseLayers.size()); i++) {
			const auto enabledLabel = "Enabled " + std::to_string(i);
			add_member_control(this, enabledLabel, terrainParams.noiseLayers[i].enabled);

			const auto frequencyLabel = "Frequency " + std::to_string(i);
			add_member_control(this, frequencyLabel, terrainParams.noiseLayers[i].frequency);

			const auto amplitudeLabel = "Amplitude " + std::to_string(i);
			add_member_control(this, amplitudeLabel, terrainParams.noiseLayers[i].amplitude);
		}

		connect_copy(add_button("Add Noise Layer")->click, cgv::signal::rebind(this, &vr_env::add_new_noise_layer));

		align("\a");
		end_tree_node_void(this);
	}
}

bool vr_env::handle(cgv::gui::event &e) { return false; }

std::string vr_env::get_type_name() const { return "vr_env"; }

void vr_env::on_set(void *member_ptr) {
	// NOTE: on_set is called every time something in the gui changes, thus we have to post a redraw event to make
	// sure any changes to the render styles ar being applied
	update_member(member_ptr);
	post_redraw();
}

void vr_env::generate_new_seed() {
	std::random_device rand_dev;
	std::mt19937 generator(rand_dev());
	std::uniform_int_distribution<int> distribution(0, 1000000);
	terrainParams.seed = distribution(generator);
	post_recreate_gui();
	post_redraw();
}

void vr_env::add_new_noise_layer() {
	terrainParams.noiseLayers.emplace_back();
	post_recreate_gui();
	post_redraw();
}

cgv::base::object_registration<vr_env> vr_env_reg("vr_env"); // NOLINT(cert-err58-cpp)
